---
search:
  boost: -0.5
---

!!! note "Important Notice for Dataset Users"
    This page is no longer relevant for users who have adopted the new Datasets feature for creating tests.
    Please refer to [the Datasets documentation](../../dataset/core-concepts/index.md)

# :octicons-container-24: Packaging for Automated Evaluation

## Introduction

In addition to analyzing and debugging model performance, we can also use the Kolena platform to create and curate
test cases and test suites. Kolena can automatically compute metrics on it for any models that have already uploaded
inferences. In this guide, we'll learn how to package our custom metrics engine such that it can be used in this
automatic evaluation process.

To enable automatic metrics computation when applicable, we need to package the metrics evaluation logic into a
Docker image that the Kolena platform can run. The following sections explain how to build this Docker image and link
it for metrics computation on the Kolena platform.

## Build Evaluator Docker Image

We will use the keypoint detection workflow we've built in the [Building a Workflow](../building-a-workflow.md)
guide to illustrate the process. Here is the project structure:

```
.
├── docker/
│   ├── build.sh
│   ├── publish.sh
│   └── Dockerfile
├── keypoint_detection/
│   ├── __init__.py
│   ├── evaluator.py
│   ├── main.py
│   └── workflow.py
├── poetry.lock
└── pyproject.toml
```

The `keypoint_detection` directory is where our workflow is defined, with evaluator logic in `evaluator.py` and
workflow data objects in `workflow.py`. The `main.py` will be the entry point where `test` is executed.

From the [workflow building guide](../building-a-workflow.md#step-4-running-tests), we know that metrics
evaluation using [`test`][kolena.workflow.test] involves a `model`, a `test_suite`, an `evaluator`, and optional
`configurations`:

```python
test(model, test_suite, evaluator, configurations=configurations)
```

!!! note "Note: test invocation"

    Ensure that `reset=True` is NOT used in the `test` method when you only want to re-evaluate metrics and do not
    have the model `infer` logic built in the image. The flag would overwrite existing inference and metrics
    results of the test suite, therefore requires re-running model `infer` on the test samples.

When executing `test` locally, the model and test suite can be initiated by user inputs. When Kolena executes `test`
under automation, this information would have to be obtained through environment variables.
Kolena sets up following environment variables for evaluator execution:

- `KOLENA_MODEL_NAME`
- `KOLENA_TEST_SUITE_NAME`
- `KOLENA_TEST_SUITE_VERSION`
- `KOLENA_TOKEN`

The main script would therefore be adjusted like code sample below.

```python title="keypoint_detection/main.py"
import os

import kolena
from kolena.workflow import test

from .evaluator import evaluate_keypoint_detection, NmeThreshold
from .workflow import Model, TestSuite


def main() -> None:
    kolena.initialize(verbose=True)

    model = Model(os.environ["KOLENA_MODEL_NAME"])
    test_suite = TestSuite.load(
        os.environ["KOLENA_TEST_SUITE_NAME"],
        os.environ["KOLENA_TEST_SUITE_VERSION"],
    )

    test(model, test_suite, evaluate_keypoint_detection, configurations=[NmeThreshold(0.05)])


if __name__ == "__main__":
    main()
```

Now that we have the main script ready, the next step is to package this script into a Docker image.

```dockerfile title="docker/Dockerfile"
FROM python:3.9-slim AS base

WORKDIR /opt/keypoint_detection/

FROM base AS builder

ARG KOLENA_TOKEN
ENV POETRY_VIRTUALENVS_IN_PROJECT=true \
    POETRY_NO_INTERACTION=1
RUN python3 -m pip install poetry

COPY pyproject.toml poetry.lock ./
COPY keypoint_detection ./keypoint_detection
RUN poetry install --only main

FROM base

COPY --from=builder /opt/keypoint_detection /opt/keypoint_detection/
COPY --from=builder /opt/keypoint_detection/.venv .venv/

ENTRYPOINT [ "/opt/keypoint_detection/.venv/bin/python", "keypoint_detection/main.py" ]
```

```shell title="docker/build.sh"
#!/usr/bin/env bash

set -eu

IMAGE_NAME="keypoint_detection_evaluator"
IMAGE_VERSION="v1"
IMAGE_TAG="$IMAGE_NAME:$IMAGE_VERSION"

echo "building $IMAGE_TAG..."

export DOCKER_BUILDKIT=1
export COMPOSE_DOCKER_CLI_BUILD=1

docker build \
    --tag "$IMAGE_TAG" \
    --file "docker/Dockerfile" \
    --build-arg KOLENA_TOKEN=${KOLENA_TOKEN} \
    .
```

This build process installs the `kolena` package, and as such needs the `KOLENA_TOKEN` environment variable to be
populated with your Kolena API key.
Follow the [`kolena` Python client](../../installing-kolena.md#initialization) guide to obtain an API key if you have not
done so.

```shell
export KOLENA_TOKEN="<kolena-api-token>"
./docker/build.sh
```

## Register Evaluator for Workflow

The final step is to publish the Docker image and associate the image with the `Keypoint Detection` workflow.

Kolena supports metrics computation using Docker image hosted on any public Docker registry or Kolena's Docker registry.
In this tutorial, we will publish our image to Kolena's Docker registry. However, the steps should be easy to adapt
to public Docker registry.

The repositories on Kolena Docker registry must be prefixed with the organization name. This is to protect unauthorized
access from unintended parties. Replace `<organization>` in `publish.sh` script with the actual organization name and
run it. This would push our Docker image to the repository and register it for the workflow.

```shell title="docker/publish.sh"
#!/usr/bin/env bash

set -eu

IMAGE_NAME="keypoint_detection_evaluator"
IMAGE_VERSION="v1"
IMAGE_TAG="${IMAGE_NAME}:${IMAGE_VERSION}"

DOCKER_REGISTRY="docker.kolena.io"
WORKFLOW="Keypoint Detection"
EVALUATOR_NAME="evaluate_keypoint_detection"
ORGANIZATION=<organization>

TARGET_IMAGE_TAG="$DOCKER_REGISTRY/$ORGANIZATION/$IMAGE_TAG"

# create repository if not exist
uv run kolena repository create --name "$ORGANIZATION/$IMAGE_NAME"

echo $KOLENA_TOKEN | docker login -u "$ORGANIZATION" --password-stdin $DOCKER_REGISTRY

echo "publishing $TARGET_IMAGE_TAG..."

docker tag $IMAGE_TAG $TARGET_IMAGE_TAG
docker push $TARGET_IMAGE_TAG

echo "registering image $TARGET_IMAGE_TAG for evaluator $EVALUATOR_NAME of workflow $WORKFLOW..."

uv run kolena evaluator register \
  --workflow "$WORKFLOW" \
  --evaluator-name "$EVALUATOR_NAME" \
  --image $TARGET_IMAGE_TAG
```

```
./docker/publish.sh
```

In `publish.sh`, we used Kolena client SDK command-line `kolena` to associate the Docker image to evaluator
`evaluate_keypoint_detection` of workflow `Keypoint Detection`. You can find out more of its usage with the `--help`
option.

## Using Automatic Metrics Evaluation

At this point, we are all set to leverage Kolena's automatic metrics evaluation capability. To see it in
action, let's first use Kolena's Studio to curate a new test case.

Head over to the [:kolena-studio-16: Studio](https://app.kolena.com/redirect/studio) and use the "Explore" tab to learn
more about the test samples from a given test case.
Select multiple test samples of interest and then go to the "Create" tab to create a new test case with the
"Create Test Case" button. You will notice there's an option to compute metrics on this new test case for applicable
models. Since we have the evaluator image registered for our workflow `Keypoint Detection`, Kolena will
automatically compute metrics for the new case if this option is checked. After the computation completes, metrics of
the new test case are immediately ready for us to analyze on the Results page.

## Conclusion

In this tutorial, we learned how to configure Kolena to automatically compute metrics when applicable, and
why it brings values to model testing and analyzing process. We can use these tools to continue improving our test
cases and our models.

## Appendix

### Evaluator runtime limits

Currently, the environment evaluator runs in does not support GPU. There is a maximum of 6 hours processing time. The
evaluation job would be terminated when the run time reaches the limit.

### Testing evaluator locally

You can verify the evaluator Docker image by running it locally:

```shell
docker run --rm \
  -e KOLENA_TEST_SUITE_NAME="${EXISTING_TEST_SUITE_NAME}" \
  -e KOLENA_TEST_SUITE_VERSION=3 \
  -e KOLENA_MODEL_NAME="example keypoint detection model" \
  -e KOLENA_WORKFLOW="Keypoint Detection" \
  -e KOLENA_TOKEN=$KOLENA_TOKEN \
  <evaluator-docker-image>
```

You can find a test suite's version on the [:kolena-test-suite-16: Test Suites](https://app.kolena.com/redirect/testing)
page. By default, the latest version is displayed.

### Using docker.kolena.io

In this tutorial, we published an evaluator container image to `docker.kolena.io`, Kolena's Docker Registry. In this
section, we'll explain how to use the Docker CLI to interact with `docker.kolena.io`.

The first step is to use `docker login` to log into `docker.kolena.io`. Using your organization's name (e.g.
`my-organization`, the part after `app.kolena.com` when you visit the app) as a username and
your API token as a password, log in with the following command:

```shell
echo $KOLENA_TOKEN | docker login --username my-organization --password-stdin docker.kolena.io
```

Once you've successfully logged in, you can use Docker CLI to perform actions on the Kolena Docker registry. For
example, to pull a previously published Docker image, use a command like:

```shell
docker pull docker.kolena.io/my-organization/<docker-image-tag>
```

If you're building Docker images for a new workflow, use the `kolena` command-line tool to create the repository on
`docker.kolena.io` first. As mentioned in [Register Evaluator for Workflow](#register-evaluator-for-workflow), the
repository must be prefixed with your organization's name.

```shell
uv run kolena repository create -n my-organization/new-evaluator
```

After the repository is created, we can use the Docker CLI to publish a newly built Docker image to `docker.kolena.io`:

```shell
docker push docker.kolena.io/my-organization/new-evaluator:v1
```

### Using Secrets in your Evaluator

If secret or sensitive data is used in your evaluation process, Kolena's secret manager can store this securely
and pass it as the environment variable `KOLENA_EVALUATOR_SECRET` at runtime.

Update the evaluator register command in `docker/publish.sh` to pass in sensitive data for the evaluator:

```shell
uv run kolena evaluator register --workflow "$WORKFLOW" \
  --evaluator-name "$EVALUATOR_NAME" \
  --image $TARGET_IMAGE_TAG \
  --secret '<your secret>'
```

### Using AWS APIs in your Evaluator

If your evaluator requires access to AWS APIs, specify the full AWS role ARN it should use in the evaluator register
command.

```shell
uv run kolena evaluator register --workflow "$WORKFLOW" \
  --evaluator-name "$EVALUATOR_NAME" \
  --image $TARGET_IMAGE_TAG \
  --aws-assume-role <target_role_arn>
```

The output of the command would look like:

```json
{
  "workflow": "Keypoint Detection",
  "name": "evaluate_keypoint_detection",
  "image": "docker.kolena.io/my-organization/keypoint_detection_evaluator:v1",
  "created": "2023-04-03 16:18:10.703 -0700",
  "secret": null,
  "aws_role_config": {
    "job_role_arn": "<Kolena AWS role ARN>",
    "external_id": "<Generated external_id>",
    "assume_role_arn": "<target_role_arn>"
  }
}
```

The response includes the AWS role ARN that Kolena will use to run the evaluator Docker image,
`aws_role_config.job_role_arn`, and the external_id, `aws_role_config.external_id`, to verify that requests are made
from Kolena.

To allow Kolena's AWS role to assume the target role in your AWS account, you need to configure the trust policy
of the target role. Here is an example of the trust policy.

```json
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": [
        "sts:AssumeRole"
      ],
      "Principal": {
        "AWS": "<Kolena AWS role ARN>"
      },
      "Condition": {
        "StringEquals": {
          "sts:ExternalId": "<External_id generated by Kolena>"
        }
      }
    }
  ]
}
```

Please refer to AWS documents for details
on [Delegate access across AWS accounts using IAM roles](https://docs.aws.amazon.com/IAM/latest/UserGuide/tutorial_cross-account-with-roles.html).

At runtime, Kolena would pass in the target role and the `external_id` in environment variables
`KOLENA_EVALUATOR_ASSUME_ROLE_ARN` and `KOLENA_EVALUATOR_EXTERNAL_ID`, respectively. The evaluator would then use
AWS assume-role to transit into the intended target role, and use AWS APIs under the new role.

```python
import os
import boto3

response = boto3.client("sts").assume_role(
    RoleArn=os.environ["KOLENA_EVALUATOR_ASSUME_ROLE_ARN"],
    ExternalId=os.environ["KOLENA_EVALUATOR_EXTERNAL_ID"],
    RoleSessionName="metrics-evaluator",
)
credentials = response["Credentials"]
```

An example of making AWS API requests under the assumed role is shown below.

```python
# use credentials to initialize AWS sessions/clients
client = boto3.client(
    "s3",
    aws_access_key_id=credentials["AccessKeyId"],
    aws_secret_access_key=credentials["SecretAccessKey"],
    aws_session_token=credentials["SessionToken"],
)
```
