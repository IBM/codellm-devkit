---
icon: kolena/diarization-workflow-16

---
# :kolena-diarization-workflow-20: LLM Powered Data Processing

Prompt extractions using LLMs enable you to perform powerful data processing activities on Kolena and
use the LLM outputs in variety of ways across the platform. We will go over the details in this document.

!!! Info
    Kolena is using on-prem models that do not require export of any customer data
    outside of data services used to run the application.

## Configuration

LLM powered property extraction can be configured from the Dataset Details page. Scroll to he bottom to create new
extractions or see existing ones.

<figure markdown>
![Access LLM Prompt Extraction](../../assets/images/prompt-extraction-access-dark.gif#only-dark)
![Access LLM Prompt Extraction](../../assets/images/prompt-extraction-access-light.gif#only-light)
<figcaption>Access LLM Prompt Extraction</figcaption>
</figure>

Prompt based extractions consist of three main components.

**Field Name**: a unique name that will be used to reference the extractions across the platform

**Model**: the model you wish to use to execute the prompt on. Currently Kolena supports
`Llamma 3.1-8b`, `Llamma 3.1-70b`, `Llamma 3.1-405b`, `Llamma 3-8b`, `Llamma 3-70b` and `Mixtral-8x7B`

**Prompt**: instructions on you wish the LLM to execute.
Using the `@` sign you can reference dynamic fields from dataset and model results in your prompt.

!!! note
    If a prompt has only references to the dataset fields, it is represented as a property of your dataset and can be
    used to create test cases.

    If a prompt had references to results and (or) datasets, it is considered a property of the results and can be used to
    create metrics.

!!! note
    When working on a new prompt, use the `Try it out` button on the prompt configurator to see a sample of 50
    extractions and tune your prompts accordingly.

!!! Example
    **1 - Translations**

    Imagine working on a dataset with text fields in a language that you are not familiar with. Using the LLM based
    extractions you can translate text fields into a language you are familiar with. An example prompt would be:

    `Please translate the following text to English: @datapoint.mandarin-text`

    **2 - Categories**

    You can use the prompt based extractions to create categories of large texts that can be used in your analysis
    of model performance. For example the following prompt generates categories base don article summaries that can
    be used to setup Test cases on Kolena:

    `Provide a category name for the following article summary:
    @datapoint.text_summary
    If you are not sure what the category is, response with "Unknown". Do NOT include additional information in your response.
    Do not use upper case in your response.`

    **3 - Evaluations**

    You can use this feature to evaluate your text based model results. For example if you are using an LLM for
    summarization tasks and want to evaluate multiple models, you can use the following prompt to assign a score
    to each model and use that score to create a metric for model evaluation:

    `Given the following article details and article summary, provide a score of 1 to 5 on completeness of the summary.
    Do NOT provide the reasoning in your response. Do not include any additional information besides the score. If the
    article summary is missing, respond with "unknown".
    Article details:@datapoint.article-details
    Article summary:@result.article-details`
