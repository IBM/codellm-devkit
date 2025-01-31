---
icon: material/microsoft-azure
---

# Connecting Cloud Storage: <nobr>:material-microsoft-azure: Azure Blob Storage</nobr>

Kolena connects with [Azure Blob Storage](https://azure.microsoft.com/en-ca/products/storage/blobs)
to load files (e.g. images, videos, documents) directly into your browser for visualization.
In this tutorial, we'll learn how to establish an integration between Kolena and
Azure Blob Storage.

To get started, ensure you have administrator access within Kolena.
Navigate to the "Integrations" tab on the [:kolena-organization-16: Organization Settings](https://app.kolena.com/redirect/organization?tab=integrations)
page and click "Add Integration", then "Azure Blob Storage".

### Step 1: Create Azure App Registration for Kolena

Azure Blob Storage integrations load resources using [shared access signatures](https://learn.microsoft.com/en-us/azure/storage/common/storage-sas-overview).
Kolena generates these signatures using delegated access to these resources.
We will generate an App registration for Kolena in Azure and then assign roles to this registration:

1. From the [Azure portal](https://portal.azure.com/#home), search for "App registrations" and navigate to this page
1. Click "New Registration"
    1. Under "Supported account types", select "Accounts in any organizational directory"
    1. Click "Register" to save the App registration
1. Click on the App registration you have created
1. Note the "Tenant ID" and "Application (client) ID"
1. Click "Certificates & secrets", then "New client secret"
    1. Click "Add" to save this secret and note the key value

### Step 2: Assign Roles to App Registration

We will assign two roles to the App registration created above:

- [Storage Blob Delegator](https://learn.microsoft.com/en-us/azure/role-based-access-control/built-in-roles#storage-blob-delegator)
  at the storage account level
- [Storage Blob Data Reader](https://learn.microsoft.com/en-us/azure/role-based-access-control/built-in-roles#storage-blob-data-reader)
  at the container level

#### Assign Storage Blob Delegator Role

1. Navigate to the storage account containing your blobs
1. Click "Access Control (IAM)"
1. Click the "Role assignments" tab
    1. Click "Add", then "Add role assignment"
1. Search for and select "Storage Blob Delegator"
1. Click on the "Members" tab, then click "Select members"
    1. Search for the App registration created in [step 1](#step-1-create-azure-app-registration-for-kolena)
    1. Click "Select"
1. Click "Review + assign" to save

#### Assign Storage Blob Data Reader role

1. From the storage account, click "Containers" under "Data Storage" and click on the container containing your blobs
1. Click "Access Control (IAM)"
1. Click the "Role assignments" tab
    1. Click "Add", then "Add role assignment"
1. Search for and select "Storage Blob Data Reader"
1. Click on the "Members" tab, then click "Select members"
    1. Search for the App registration created in [step 1](#step-1-create-azure-app-registration-for-kolena)
    1. Click "Select"
1. Click "Review + assign" to save

### Step 3: Save Integration in Kolena

Return to Kolena and fill in the fields for the Integration and then click "Save".

| Field                    | Description                                                                                                                                                                                       |
| ------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Tenant ID                | The Tenant ID of the App registration created in [step 1](#step-1-create-azure-app-registration-for-kolena)                                                                                       |
| Client ID                | The Application (client) ID of the App registration created in [step 1](#step-1-create-azure-app-registration-for-kolena)                                                                         |
| Client Secret            | The secret key for the App registration created in [step 1](#step-1-create-azure-app-registration-for-kolena)                                                                                     |
| Storage Account Name     | The storage account in Azure you wish to connect to                                                                                                                                               |
| Storage Blob EndpointURL | The endpoint for accessing the storage account. Can be found in "Endpoints" under "Settings" for your storage account. Usually of the form `https://<storage-account-name>.blob.core.windows.net` |

## Appendix

### Provide CORS access to Kolena

In some scenarios, CORS permissions are required for Kolena to render content from your bucket.

To configure CORS access, navigate to your storage account in the Azure portal and follow these steps:

1. Click "Resource Sharing (CORS)" under "Settings"
1. Add `https://app.kolena.com` and `https://app.kolena.io` as allowed origins with `GET` as an allowed method
