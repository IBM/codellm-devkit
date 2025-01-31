---
icon: simple/amazons3
---

# Connecting Cloud Storage: <nobr>:simple-amazons3: Amazon S3</nobr>

Kolena connects with [Amazon S3](https://aws.amazon.com/s3/) to load files (e.g. images, videos, documents) directly
into your browser for visualization. In this tutorial, we'll learn how to establish an integration between Kolena and
Amazon S3.

To get started, ensure you have administrator access within Kolena.
Navigate to the "Integrations" tab on the [:kolena-organization-16: Organization Settings](https://app.kolena.com/redirect/organization?tab=integrations)
page and click "Add Integration", then "Amazon S3".

### Step 1: Select Integration Scope

Amazon S3 integrations load bucket objects using [pre-signed URLs](https://docs.aws.amazon.com/AmazonS3/latest/userguide/ShareObjectPreSignedURL.html).
Kolena generates these URLs by temporarily [assuming an IAM role](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_roles_use.html)
that has access to the specified bucket(s).

By default, Kolena will assume this role when loading objects from any permitted bucket.
Alternatively, if you wish for Kolena to assume this role only while loading objects from one bucket, uncheck
"Use role by default for all permitted buckets?" and specify the name of an S3 bucket.

Click "Next".

!!! note "Note: Scoping Integrations"

    If "Use role by default for all permitted buckets?" is selected, Kolena will load any locators beginning with
    `s3://` by assuming the role configured for this Integration and generating a presigned URL.

    Scoping the Integration to one bucket (e.g. `my-bucket`) means Kolena will only assume the role when generating
    presigned URLs for locators of the form `s3://my-bucket/*`.

### Step 2: Create an Access Policy in AWS

If you selected "Use role by default for all permitted buckets?" in the previous step, you must now choose which buckets
Kolena is permitted to load objects from.
Enter these bucket names.

When you have entered the bucket names, you will see an "Access Policy JSON" rendered to your page.
Copy this JSON.

!!! note "Note: IAM Write Permission Required"

    You will require IAM write permissions within your AWS account to perform the next step.

In your AWS console, navigate to the
<a target="_blank" href="https://console.aws.amazon.com/iamv2/home#/policies">IAM policies page</a> and follow these steps:

1. Click the "Create Policy" button and select the "JSON" tab.
2. Paste the "Access Policy JSON" copied previously.
3. Click through the "Next" buttons, adding the desired name, description, and tags.

!!! note "Note: Enabling File Versioning"

    If you have previously configured an S3 integration and would like to enable file versioning, update your existing
    policy to include permissions for the `s3:GetObjectVersion` Action.

### Step 3: Create a Role For Kolena to Assume

Return to Kolena and copy the "Trust Policy JSON".

In your AWS console, navigate to the
<a target="_blank" href="https://console.aws.amazon.com/iamv2/home#/roles">IAM roles page</a> and follow these steps:

1. Click the "Create role" button and select "Custom trust policy".
2. Paste the "Trust Policy JSON" you copied above and click "Next".
3. Search for and select the access policy created in [step 2](#step-2-create-an-access-policy-in-aws). Click "Next".
4. Provide a role name and review the permissions, then click "Create role".

**Copy the role's ARN for use in the next step.**

### Step 4: Save Integration

Return to Kolena and fill in the remaining fields for the Integration and then click "Save".

| Field    | Description                                                                         |
| -------- | ----------------------------------------------------------------------------------- |
| Role ARN | The ARN of the role created in [step 3](#step-3-create-a-role-for-kolena-to-assume) |
| Region   | The region your buckets will be accessed from (e.g. `us-east-1`)                    |

## Appendix

### Allow CORS Access to Bucket

In some scenarios, CORS permissions are required for Kolena to render content from your bucket.

To configure CORS access, navigate to your S3 bucket inside your AWS console and follow these steps:

1. Click on the "Permissions" tab and navigate to the "Cross-origin resource sharing (CORS)" section.
2. Click "Edit" and add the following JSON snippet:

```json
[
  {
    "AllowedHeaders": ["*"],
    "AllowedMethods": ["GET"],
    "AllowedOrigins": [
      "https://app.kolena.com",
      "https://app.kolena.io"
    ],
    "ExposeHeaders": []
  }
]
```
