---
icon: kolena/flag-16
hide:
  - navigation
  - toc
---

# :kolena-flag-20: Frequently Asked Questions

This page answers common questions about Kolena and how to use it to test ML models.

If you don't see your question here, please reach out to us on Slack or at
[contact@kolena.com](mailto:contact@kolena.com)!

## About Kolena

??? faq "What data types does Kolena support?"

    Testing in Kolena is fully customizable and supports computer vision, natural language processing, and structured
    data (tabular, time series) machine learning models. This includes images, documents, videos, 3D models and point
    clouds, and more.

    See the available data types in [`kolena.workflow.TestSample`][kolena.workflow.TestSample], and the available
    annotation types in [`kolena.workflow.annotation`][kolena.workflow.annotation.Annotation].

    We're constantly adding new data types and annotation types — if you don't see what you're looking for, reach out
    to us and we'll happily extend our system to support your use case.

??? faq "Do I have to upload my images, documents, or files to Kolena?"

    No. Kolena doesn't store your data (images, videos, documents, 3D assetes, etc.) directly, only URLs pointing to
    the right location in a cloud bucket or internal infrastructure that you own.

    While onboarding your team, we'll discuss what access restrictions are necessary for your data and select the right
    integration solution. As one example, as a part of the integration we might restrict access to files registered with
    Kolena to only users on your corporate VPN.

    We support a variety of integration patterns depending on your organization's requirements and security stance.
    [Get in touch with us to discuss details](https://www.kolena.com/schedule-a-demo)!

??? faq "Do I have to upload my models to Kolena?"

    No. Tests are always run in your environment using the [`kolena` Python client](../installing-kolena.md), and you never
    have to package or upload models to Kolena.

??? faq "Where does Kolena fit into the MLOps development life cycle?"

    Kolena is primarily a testing (or "offline evaluation") platform, coming _after_ training and _before_ deployment.
    We believe that increased emphasis on this offline evaluation segment of the model development life cycle can save
    effort upstream in the data collection and training process as well as prevent headaches downstream in deployment.

## Using Kolena

??? faq "How do I generate an API token?"

    Generate an API token by visiting the [:kolena-developer-16: Developer](https://app.kolena.com/redirect/developer)
    page, located at the bottom of the lefthand sidebar, then copy/paste the shell snippet to set this token as
    `KOLENA_TOKEN` in your environment.

??? faq "How many API tokens can I generate?"

    API tokens are scoped to your username. Each user is limited to one valid token at a time — generating a new token
    on the [:kolena-developer-16: Developer](https://app.kolena.com/redirect/developer) page invalidates any previous
    token generated for your user.

    To retrieve a service user API token that is not scoped to a specific username, please reach out to us on Slack or
    at [contact@kolena.com](mailto:contact@kolena.com).

??? faq "My data is being ingested in the wrong format for a CSV exported using `pandas.DataFrame.to_csv`"

    `pandas.DataFrame.to_csv` does not always handle object serialization seamlessly. Please reference
    [`dataframe_to_csv`](../reference/io.md#kolena.io.dataframe_to_csv) for a drop-in replacement.

??? faq "Does Kolena support file versioning?"

    If you are using Amazon S3 or Google Cloud Storage, Kolena supports file versioning for any linked `locator` files.
    This includes [test samples](../reference/workflow/test-sample.md), [assets](../reference/workflow/asset.md), as
    well as certain [annotation](../reference/workflow/annotation.md) types. Simply enable bucket versioning on your
    [S3](https://docs.aws.amazon.com/AmazonS3/latest/userguide/Versioning.html) or
    [GCS](https://cloud.google.com/storage/docs/object-versioning) bucket and make sure to pass the `versionId` (S3)
    or `generation` (GCS) as part of the `locator`.

    For more information, see the examples in [`kolena.workflow.TestSample`][kolena.workflow.TestSample].

??? faq "How can I add new users to my organization?"

    Administrators for your organization can add new users and grant users administrator privileges by visiting the
    [:kolena-organization-16: Organization Settings](https://app.kolena.com/redirect/organization) page and adding
    entries to the **Authorized Users** table.

    Note that this page is only visible for organization administrators.

??? faq "Who are administrators for my organization?"

    Certain members of each organization have administrator privileges.
    These administrators can manage users access and privileges,
    as well as configure Integrations.

    You may become an administrator by having an existing administrator grant you privileges
    using the [:kolena-organization-16: Organization Settings](https://app.kolena.com/redirect/organization?tab=users) page.

??? faq "I'm new to Kolena — how can I learn more about the platform and how to use it?"

    On each page, there is a button with the :kolena-learning-16: icon next to the page title. Click on this
    button to bring up a detailed tutorial explaining the contents of the current page and how it's used.

??? faq "How can I report a bug?"

    If you encounter a bug when using the `kolena` Python client or when using [app.kolena.com](https://app.kolena.com),
    message us on Slack, email your support representative or [contact@kolena.com](mailto:contact@kolena.com), or
    [open an issue on the `kolena` repository](https://github.com/kolenaIO/kolena/issues) for Python-client-related
    issues.

    Please include any relevant stacktrace or platform URL when reporting an issue.
