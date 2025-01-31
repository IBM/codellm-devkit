---
description: Prompting LLMs to produce a metric for hallucinations
---

# Prompt-based Hallucination Metric

The prompt-based hallucination metric uses a strong Large Language Model (LLM) judge, such as
[OpenAI's GPT-4](https://openai.com/gpt-4), to detect hallucination. The metric can be computed in various ways
depending on the prompt given to the LLM. Depending on the prompt technique used, the metric can be a boolean value,
where `True` indicates hallucination, or a numeric value ranging from 0 to 1, where 0 indicates no hallucination
and 1 indicates definite hallucination.

## Implementation Details

The main prompting techniques we will focus on in this guide are:

1. [**Chain-of-Thought Prompt**](#chain-of-thought-prompt) - This prompting technique asks the judging model to answer
with a `yes` or `no` if the generated response contains hallucination. When the judge detects hallucination, the
[Chain-of-Thought (CoT)](https://www.promptingguide.ai/techniques/cot) prompting technique is used to improve the
model's reasoning capabilities.
2. [**Self-Consistency Prompt**](#self-consistency-prompt) - This technique involves prompting the judging model
multiple times with the same prompt asking it to detect hallucinations and aggregating the outputs to obtain a score,
known as a hallucination score.

In practice, we recommend using a combination of the self-consistency prompting technique and the CoT technique.

In the following section, we will compute the prompt-based hallucination metric using these two
prompting techniques on example responses. We will use [GPT-4](https://openai.com/gpt-4) as the judging model, as it
is currently the most effective at detecting hallucinations.

### Example

Given the following set of ground truths and inferences, we will compute the hallucination metric using the two
prompting techniques.

| Ground Truth | Inference |
| --- | --- |
| `The duck crossed the road` | `The duck did not cross the road` |
| `The duck crossed the road` | `The animal crossed the road` |
| `The duck crossed the road` | `The duck may not be the one who crossed the road` |

To make API requests to OpenAI GPT models, you need to install the OpenAI Python library and set up your API key. If you
don't have a secret key yet, you can create one on [OpenAI's API key page](https://platform.openai.com/account/api-keys).

```
pip install openai
export OPENAI_API_KEY=`your-api-key-here`
```

After setting up an API key, let's prompt the judging model of your choice for each pair of ground truth and inference.

#### Chain-of-Thought Prompt

Using the CoT prompting technique, we can prompt the judging model to evaluate hallucination in a boolean format and
ask for its reasoning. Here is an example of the CoT prompt technique:

```
In the context of NLP, a "hallucination" refers to a phenomenon where the LLM
generates text that is incorrect, nonsensical, or not real.

Given two texts, where the first one is a perfect answer, and the second one
is a generated answer, if the generated answer is considered a "hallucination",
return "yes". Otherwise, simply return "no". If yes, on the next line, explain
in about ten words why there is a hallucination by thinking step by step.
```

!!! info "Tips on Prompt for Question Answering Workflow"

    By providing more context along with the expected and generated answers, you can achieve more accurate evaluation.
    We recommend adding the question to the answer in the prompt.

    If you are working on a closed-domain or retrieval-augmented generation (RAG) type dataset, it is recommended
    to provide the reference or retrieved text along with the question and answers to provide more context to detect
    hallucinations.

??? info "How to Prompt `gpt-4`"

    ```
    import os
    from openai import OpenAI

    openai = OpenAI(api_key=os.environ["OPENAI_API_KEY"])

    PROMPT = """
    In the context of NLP, a "hallucination" refers to a phenomenon where the LLM generates text that is incorrect, \
    nonsensical, or not real.

    Given two texts, where the first one is a perfect answer, and the second one is a generated answer, if the
    generated answer is considered a "hallucination", return "yes". Otherwise, simply return "no".
    If yes, on the next line, explain in about ten words why there is a hallucination by thinking step by step.
    """

    response = openai.chat.completions.create(
        model="gpt-4-1106-preview",
        messages=[
            {
                "role": "user",
                "content": PROMPT,
            },
            {
                "role": "assistant",
                "content": "Certainly! Please provide me with the texts for evaluation.",
            },
            {
                "role": "user",
                "content": f"Perfect Answer: {ground_truth}" f"\n\nGenerated Answer: {inference}",
            },
        ],
        temperature=0.5,
        max_tokens=50,
        top_p=1,
        frequency_penalty=0,
        presence_penalty=0,
    )

    response = str(response.choices[0].message.content)
    ```

| Ground Truth | Inference | GPT-4 Judge Response | Reasoning |
| --- | --- | --- | --- |
| `The duck crossed the road` | `The duck did not cross the road` | `yes` | `The generated answer contradicts the perfect answer directly.` |
| `The duck crossed the road` | `The animal crossed the road` | `no` | `-` |
| `The duck crossed the road` | `The duck may not be the one who crossed the road` | `no` | `-` |

Based on GPT-4's response, we can identify any potential hallucination in each generated answer.

#### Self-Consistency Prompt

In the previous section, we learned about the CoT prompting technique. In this section, let's repeat the same prompt
five times and measure the hallucination score. If you are unsure how many times to repeat, starting with five is a good
option. The hallucination score is calculated as the rate of the number of times the judging model detected
hallucination to the total number of instances:

$$
\text{hallucination score} = \frac{\text{number of hallucinating instances}}{\text{total number of instances}}
$$

| Ground Truth | Inference | GPT-4 Judge Responses | Hallucination Score |
| --- | --- | --- | --- |
| `The duck crossed the road` | `The duck did not cross the road` | [`yes`, `yes`, `yes`, `yes`, `yes`] | 1.0 |
| `The duck crossed the road` | `The animal crossed the road` | [`no`, `no`, `no`, `no`, `no`] | 0.0 |
| `The duck crossed the road` | `The duck may not be the one who crossed the road` | [`no`, `no`, `yes`, `yes`, `no`] | 0.4 |

Based on GPT-4's response, the inference in the first example pair is definitely hallucinating, and the one in the
second pair is factually consistent. However, the inference in the last pair seems to be neutral or inconclusive, as the
judging model predicted `yes` twice out of the five times it was prompted.

## Limitations and Biases

1. **Cost** - Running a large model entails significant expenses. The cost of operating an API model such as GPT-4 is
determined by the number of tokens used. However, if you employ your own model as a judging model, the payment might not
be calculated based on the token count; nevertheless, there will be additional costs for hardware, computation and
maintenance. It is essential to remember that monetary expenses are not the sole consideration. You should also take
into account the inference time of the judging model.

2. **Privacy and Security** - To achieve desirable results, you need access to a sufficiently performant LLM. However,
using GPT-4 or similar models through an API can hold privacy and security concerns when datasets are meant to be kept
private.

While using LLMs to detect hallucinations has limitations, it also offers advantages:

1. **Improved Accuracy** - LLM prompt-based evaluation is one of the state-of-the-art techniques being used for
detecting hallucinations in LLMs.

2. **Explainability** - By using a performant LLM as a judging model, it can provide explanations for flagging a
response as a hallucination. These explanations can help us understand the reasoning behind the hallucination detection
of a given prompt.
