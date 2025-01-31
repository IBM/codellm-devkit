---
icon: material/microsoft-azure
---

# Connecting External LLM Endpoint: <nobr>:material-microsoft-azure: Azure OpenAI</nobr>

Kolena connects with [Azure OpenAI](https://azure.microsoft.com/en-us/products/ai-services/openai-service)
to leverage your hosted LLM endpoint to do data enrichment and result evaluation.
In this tutorial, we'll learn how to establish an integration between Kolena and your Azure OpenAI deployment.

To get started, ensure you have administrator access within Kolena.
Navigate to the "Integrations" tab on the [:kolena-organization-16: Organization Settings](https://app.kolena.com/redirect/organization?tab=integrations)
page under the "LLM Integration" section and click "Add Integration", then "Azure OpenAI".

Note: By default, Kolena will make up to 50 concurrent requests to your Azure OpenAI deployment to speed things up. You
can adjust this limit using the `Concurrent Worker Limit` field of the integration setting based on your need and Azure
OpenAI deployment capacity.

### Step 1: Create Azure LLM Registration for Kolena

We will generate an App registration for Kolena in Azure and then assign roles to this registration:

1. From the [Azure portal](https://portal.azure.com/#home), search for "App registrations" and navigate to this page
2. Click "New Registration"
    1. Under "Supported account types", select "Accounts in any organizational directory"
    2. Click "Register" to save the App registration
3. Click on the App registration you have created
4. Note the "Tenant ID" and "Application (client) ID"
5. Click "Certificates & secrets", then "New client secret"
    1. Click "Add" to save this secret and note the key value

### Step 2: Assign Roles to App Registration

We will assign a role to the App registration created above:

1. Navigate to the openai resource containing your OpenAI llm deployment
2. Search for "Access control" on the left panel
3. Click "Role assignments"
    1. Click "Add", then "Add role assignment"
    2. Search for and select [Cognitive Services OpenAI User](https://learn.microsoft.com/en-us/azure/ai-services/openai/how-to/role-based-access-control#cognitive-services-openai-user),
       this will allow Kolena to make requests to your OpenAI deployment
4. Click on the "Members" tab, then click "Select members"
    1. Search for the App registration created in [step 1](#step-1-create-azure-llm-registration-for-kolena)
    2. Click "Select"
5. Click "Review + assign" to save

### Step 3: Save Integration in Kolena

Return to Kolena and fill in the fields for the Integration and then click "Save".

| Field                   | Description                                                                                                                       |
|-------------------------|-----------------------------------------------------------------------------------------------------------------------------------|
| Tenant ID               | The Tenant ID of the App registration created in [step 1](#step-1-create-azure-llm-registration-for-kolena)                       |
| Client ID               | The Application (client) ID of the App registration created in [step 1](#step-1-create-azure-llm-registration-for-kolena)         |
| Client Secret           | The secret key for the App registration created in [step 1](#step-1-create-azure-llm-registration-for-kolena)                     |
| Endpoint                | The Azure OpenAI endpoint, it can be find under `Resource Management/Keys and Endpoint` section in Azure OpenAI console           |
| Model Name              | The Name of the model to be displayed on kolena                                                                                   |
| Deployment Name         | The deployment name for the model to be deployed on Kolena Resource `Management/Model Deployment` section in Azure OpenAI console |
| Support Image           | Whether this hosted model has vision capabilities                                                                                 |
| Concurrent Worker Limit | The number of concurrent API requests we are allowed to make to your hosted LLM endpoint per Prompt Extraction Pipeline run       |
