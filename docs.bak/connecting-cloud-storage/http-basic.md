---
icon: kolena/globe-network-16
---

# Connecting Cloud Storage: <nobr>:kolena-globe-network-20: HTTP Basic</nobr>

Kolena connects with systems that utilize HTTP basic authentication to load files (e.g. images, videos, documents) directly
into your browser for visualization. In this tutorial, we'll learn how to establish an integration between Kolena and a
file-serving system that utilizes HTTP basic authentication.

To get started, ensure you have administrator access within Kolena.
Navigate to the "Integrations" tab on the
[:kolena-organization-16: Organization Settings](https://app.kolena.com/redirect/organization?tab=integrations)
page and click "Add Integration", then "HTTP Basic".

### Step 1: Save Integration on Kolena

On the [Integrations tab](https://app.kolena.com/redirect/organization?tab=integrations),
fill in the fields for the integration and then click "Save".

| Field | Description |
|---|---|
| URL Origin | The origin of the domain you wish to load data from. Ensure you omit the protocol (e.g. `https://`) |
| Username | The username for your http basic auth system |
| Password | The password (optional) for your http basic auth system |

Any locators beginning with `https://<URL Origin>` will be loaded using this integration.
