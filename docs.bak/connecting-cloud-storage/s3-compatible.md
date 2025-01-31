---
icon: simple/minio
subtitle: MinIO, Oracle, Hitachi
---

# Connecting Cloud Storage: <nobr>:simple-minio: S3-Compatible APIs</nobr>

Kolena connects with any S3-compatible system to load files (e.g. images, videos, documents) directly into your browser
for visualization. Supported systems include:

- [:simple-minio: MinIO](https://min.io)
- [:simple-oracle: Oracle Object Storage](https://docs.oracle.com/en-us/iaas/Content/Object/Tasks/s3compatibleapi.htm)
- [:simple-hitachi: Hitachi Content Platform (HCP) for cloud scale](https://knowledge.hitachivantara.com/Documents/Storage/HCP_for_Cloud_Scale)

In this tutorial, we'll learn how to establish an integration between Kolena and a storage system implementing an
S3-compatible API.

To get started, ensure you have administrator access within Kolena.
Navigate to the "Integrations" tab on the
[:kolena-organization-16: Organization Settings](https://app.kolena.com/redirect/organization?tab=integrations)
page and click "Add Integration", then "MinIO".

Steps performed outside of Kolena are shown for a subset of possible S3-compatible systems.
You may need to consult documentation for your provider to perform equivalent steps.

### Step 1: Create a Service User for Kolena

=== "`MinIO`"

    ```shell
    mc admin user add <deployment_alias> <kolena_user> <secret_access_key>
    ```

### Step 2: Create an Access Policy

Create a policy to allow read access for a bucket or set of buckets.

Save the following JSON policy to a file called `/tmp/kolena-policy.json`,
replacing `s3://share-with-kolena` with the appropriate bucket(s):

```json
{
    "Version": "2012-10-17",
    "Statement": [{
        "Sid": "S3ListBucket",
        "Effect": "Allow",
        "Action": [
            "s3:GetObject",
            "s3:ListBucket"
        ],
        "Resource": [
            "arn:aws:s3:::share-with-kolena",
            "arn:aws:s3:::share-with-kolena/*"
        ]
    }]
}
```

!!!note "Note: Bucket names"

    Please note that bucket names must follow [S3 naming rules](https://docs.aws.amazon.com/AmazonS3/latest/userguide/bucketnamingrules.html)

Next, create the policy and attach the policy to the service user created in [step 1](#step-1-create-a-service-user-for-kolena):

=== "`MinIO`"

    ```shell
    mc admin policy create <deployment_alias> kolenaread /tmp/kolena-policy.json
    mc admin policy attach <deployment_alias> kolenaread --user <kolena_user>
    ```

### Step 3: Save Integration on Kolena

Return to the Kolena platform [Integrations tab](https://app.kolena.com/redirect/organization?tab=integrations).

By default, any locators beginning with `s3://` will be loaded using this integration.

!!!note "Note: scoping integrations"

    Optionally, each integration can be scoped to a specific bucket such that
    only locators of the pattern `s3://<specific-bucket>/*` will be loaded using the integration.
    This can be necessary if multiple integrations are required.
    Unchecking "Apply to all buckets by default?" and specifying a bucket will enable this behavior.

Fill in the fields for the integration and then click "Save".

| Field | Description |
|---|---|
| Access Key Id | The username (`<kolena_user>`) of the user created in [step 1](#step-1-create-a-service-user-for-kolena) |
| Secret Access Key | The secret key (`<secret_access_key>`) of the user created in [step 1](#step-1-create-a-service-user-for-kolena) |
| Endpoint | The hostname or IP address of your S3-compatible service |
| Port | The optional port to access your S3-compatible service |
| Region | The region your buckets will be accessed from |
