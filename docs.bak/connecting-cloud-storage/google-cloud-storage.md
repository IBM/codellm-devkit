---
icon: simple/googlecloud
---

# Connecting Cloud Storage: <nobr>:simple-googlecloud: Google Cloud Storage</nobr>

Kolena connects with [Google Cloud Storage](https://cloud.google.com/storage) to load files (e.g. images, videos,
documents) directly into your browser for visualization. In this tutorial, we'll learn how to establish an integration
between Kolena and Google Cloud Storage.

To get started, ensure you have administrator access within Kolena.
Navigate to the "Integrations" tab on the
[:kolena-organization-16: Organization Settings](https://app.kolena.com/redirect/organization?tab=integrations)
page and click "Add Integration", then "Google Cloud Storage".

!!! note "Connecting multiple projects"
    Google Cloud Storage allows you to set up access to multiple projects and buckets within them using a single integration.
    Therefore, Kolena supports a single integration with Google Cloud Storage.

### Step 1: Save Integration to Create a Service Account

From the [Integrations tab](https://app.kolena.com/redirect/organization?tab=integrations), saving a Google Cloud Storage
integration will create a service account.
Upon creation, the integration's `client_email` will be used to provide
Kolena permission to load data from your Google Cloud Storage buckets.

### Step 2: Grant Service Account Read Access

Within your Google Cloud Platform console, navigate to the bucket that contains your images.
Click on the permissions tab.
Click the "Grant Access" button and grant the service account created in
[step 1](#step-1-save-integration-to-create-a-service-account) the `Storage Object Viewer` role.
The `Storage Object Viewer` role offers the following permissions:

- Grants access to view objects and their metadata, excluding ACLs.
- Grants access to list the objects in a bucket.

### Step 3: Provide CORS Access

Create a json file `cors.json` with the following content:

```json
[
  {
    "origin": [
      "https://app.kolena.com",
      "https://app.kolena.io"
    ],
    "method": ["GET"],
    "responseHeader": ["Content-Type"],
    "maxAgeSeconds": 3600
  }
]
```

Ensure you have `gsutil` installed.
Then provide CORS access to Kolena for your bucket by running the following command:

```
gsutil cors set cors.json gs://<my-bucket>
```
