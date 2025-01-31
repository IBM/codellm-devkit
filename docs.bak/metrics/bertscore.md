---
description: Embeddings-based textual similarity for NLP tasks
---

# BERTScore

BERTScore is a metric used in NLP workflows to measure textual similarity between candidate texts and reference texts.
Unlike [BLEU](bleu.md), [ROUGE](rouge-n.md), and traditional n-gram similarity measures, it leverages pretrained BERT
embeddings to capture the semantic and contextual information of words and phrases in both the candidate and reference
texts. This approach makes BERTScore more effective at assessing the quality of candidate text because it considers not
only exact word matches but also the overall meaning, fluency, and order of the output.

!!!example
    To see and an example of Bert Score, checkout the
    [CNN-DailyMail on app.kolena.com/try.](https://app.kolena.io/try/dataset/standards?datasetId=39&models=N4IglgJiBcCcA0IDGB7AdgMzAcwK4CcBDAFzHRlEhgEYBfWoA&models=N4IglgJiBcAcA0IDGB7AdgMzAcwK4CcBDAFzHRlEhgEYBfWoA&metricGroupVisibilities=N4IgbglgzhBGA2BTEAuALgJwK6IDQgFtFMIBjKVAbVEhgWXW0QF9cbo4lVMdX26ujXm3Ad63Jn1ECGPFgF1mQA)

??? question "Recall: BERT & Textual Embeddings"
    BERT (**B**idirectional **E**ncoder **R**epresentations from **T**ransformers) is a popular language model used to
    generate embeddings from words and phrases. Textual embeddings are learned dense token representations that capture
    the semantic and contextual information of words and phrases in a continuous vector space. In a perfect embedding
    space, similar words are grouped together while words that are semantically different are distanced. For a deeper
    dive into BERT and textual embeddings, feel free to refer to the
    [original paper](https://arxiv.org/pdf/1810.04805.pdf).
    <center>![Embedding Space Image](../assets/images/metrics-bert-vector-space-light.svg#only-light){: style="height:20em;width:auto"}</center>
    <center>![Embedding Space Image](../assets/images/metrics-bert-vector-space-dark.svg#only-dark){: style="height:20em;width:auto"}</center>

## Implementation Details

BERTScore is a collection of three metrics — BERT-Precision, BERT-Recall, and BERT-F1. As the names imply,
BERT-Precision measures how well the candidate texts avoid introducing irrelevant content. BERT-Recall measures how well
the candidate texts avoid omitting relevant content. BERT-F1 is a combination of both Precision and Recall to measure
how well the candidate texts capture and retain relevant information from the reference texts.

### Calculating BERTScore

Given a reference sentence, $x = \langle x_1, x_2, ..., x_n \rangle$, and candidate sentence,
$\hat{x} = \langle\hat{x}_1, \hat{x}_2, ..., \hat{x}_m\rangle$, we first use BERT to generate a sequence of word
embeddings for both reference and candidate sentences.

<!-- markdownlint-disable MD013 -->

$$
\begin{align}
    & BERT(\langle x_1, x_2, ..., x_n \rangle) = \langle \mathbf{x_1}, \mathbf{x_2}, ..., \mathbf{x_n} \rangle \\
    & BERT(\langle \hat{x}_1, \hat{x}_2, ..., \hat{x}_m \rangle) = \langle \mathbf{\hat{x}_1}, \mathbf{\hat{x}_2}, ..., \mathbf{\hat{x}_m} \rangle
\end{align}
$$

<!-- markdownlint-enable MD013 -->

<center><p style="font-size:small;">Note that we will use <b>bold</b> text to indicate vectors, like a word embedding</p></center>

To measure the similarity between two individual embeddings, we will use the cosine similarity which is defined as:

$$
\text{similarity}(\mathbf{x_i}, \mathbf{\hat{x}_j}) = \frac{\mathbf{x_i}^\top \mathbf{\hat{x}_j}}{||\mathbf{x_i}||\space||\mathbf{\hat{x}_j}||}
$$

which simply reduces to $\mathbf{x_i}^\top \mathbf{\hat{x}_j}$ since both $\mathbf{x_i}$ and $\mathbf{\hat{x}_j}$ are
pre-normalized. With these definitions, we can now calculate the BERT-precision, BERT-recall, and BERT-F1.

#### BERT-Precision

<!-- markdownlint-disable MD013 -->

$$
P_\text{BERT} = \frac{1}{|\hat{x}|} \sum_{\mathbf{\hat{x}_j} \in \hat{x}} \underbrace{\max_{\mathbf{x_i} \in x}\overbrace{\mathbf{x_i}^\top \mathbf{\hat{x}_j}}^\text{cosine similarity}}_\text{greedy matching}
$$

<!-- markdownlint-enable MD013 -->

Though the formula may seem intimidating, BERT-precision is conceptually similar to the
[precision formula](precision.md), but uses greedy matching to maximize the similarity score between a reference word
and the current candidate word. This is because the language domain can have multiple words that are similar in context
to the ground truth, and the words of a sentence can be arranged in different ways while preserving the same meaning —
thus, why we use greedy matching.

#### BERT-Recall

<!-- markdownlint-disable MD013 -->

$$
R_\text{BERT} = \frac{1}{|x|} \sum_{\mathbf{x_i} \in x} \underbrace{\max_{\mathbf{\hat{x}_j} \in \hat{x}}\overbrace{\mathbf{x_i}^\top \mathbf{\hat{x}_j}}^\text{cosine similarity}}_\text{greedy matching}
$$

<!-- markdownlint-enable MD013 -->

Once again, the BERT-recall is conceptually similar to the [recall formula](recall.md). Note that we flip $\hat{x}$ with
$x$ when calculating recall.

#### BERT-F1

$$
F_\text{BERT} = 2 \times \frac{P_\text{BERT} \times R_\text{BERT}}{P_\text{BERT} + R_\text{BERT}}
$$

The formula is the same as the [F1-score formula](f1-score.md), replacing the precision and recall components with
BERT-precision and BERT-recall.

In a more advanced implementation of BERTScore, extra steps are taken to finetune the metric. These include:

1. Applying an "importance factor" to rare words so that the score weighs keywords more so than words like "it", "as",
and "the".
2. Rescaling the score such that it lies between 0 and 1 in practical use cases. Although the score already lies between
0 and 1 in theory, it has been observed to lie between a more limited range in practice.

[![Bert Computation](../assets/images/metrics-bert-computation-light.svg#only-light)](https://arxiv.org/pdf/1810.04805.pdf)
[![Bert Computation](../assets/images/metrics-bert-computation-dark.svg#only-dark)](https://arxiv.org/pdf/1810.04805.pdf)

### Python Implementation

There are many packages that implement the BERTScore metric, making the implementation quick and simple.

1. [HuggingFace](https://huggingface.co/spaces/evaluate-metric/bertscore) - HuggingFace provides a comprehensive
BertScore module with [140+ different BERT models](https://docs.google.com/spreadsheets/d/1RKOVpselB98Nnh_EOC4A2BYn8_201tmPODpNWu4w7xI/edit#gid=0)
to choose from,  allowing you to find the perfect balance between efficiency and accuracy.
2. [TorchMetrics](https://torchmetrics.readthedocs.io/en/stable/text/bert_score.html) - TorchMetrics provides a similar
BertScore wrapper with the same functionality as HuggingFace.
3. [bert-score](https://pypi.org/project/bert-score/) - bert-score is another package that can be used. It provides
similar functionality as HuggingFace as well.

There are many packages used to calculate BERTScore, and it is up to the user to choose their preferred package based
on their existing workflow.

### Interpretation

BERTScore (Precision, Recall, F1) scores lie between the range of 0 and 1, with 0 representing no semantic similarity,
and 1 representing a perfect semantic match between candidate and reference texts. However, interpreting the metric is
completely subjective based on your task. On some tasks, a BERT-F1 of 0.9 may be excellent, whereas a BERT-F1 of 0.8 may
be excellent for another. Generally speaking, a higher BERTScore is desirable.

## Example

To showcase the value of BERTScore, let's consider the following candidate and reference texts:

??? example "Semantically Similar Texts"

    <!-- markdownlint-disable MD013 -->

    | Candidate Text | Reference Text |
    | --- | --- |
    | The sun set behind the mountains, casting a warm, orange glow across the horizon. | As the mountains obscured the sun, a warm, orange glow painted the horizon. |
    | She sipped her coffee and gazed out of the window, lost in thought on a rainy afternoon. | Lost in thought on a rainy afternoon, she sipped her coffee and stared out of the window. |
    | The adventurous explorer trekked through the dense jungle, searching for hidden treasures. | In search of hidden treasures, the intrepid explorer ventured through the dense jungle. |
    | Laughter echoed through the park as children played on the swings and slides. | Children's laughter filled the park as they enjoyed the swings and slides. |
    | The old bookstore was filled with the scent of well-worn pages, a haven for book lovers. | A haven for book lovers, the old bookstore exuded the fragrance of well-read pages. |

    <!-- markdownlint-enable MD013 -->

    Using the following code and the [`bert-score`](https://pypi.org/project/bert-score/) package:

    ```python
    from bert_score import score

    candidates = [...]
    references = [...]
    precision, recall, f1 = score(c, r, lang='en') # using the default `roberta-large` BERT model

    precision, recall, f1 = precision.mean(), recall.mean(), f1.mean()
    ```

    We get the following BertScores: $P_\text{BERT} = 0.9526, R_\text{BERT} = 0.9480, F_\text{BERT} = 0.9503$. This is
    in-line with human judgement, as the reference and candidate texts are semantically very similar.

    However, if we were to calculate the BLEU score given these candidates and references, BLEU would yield a
    sub-optimal score of $\text{BLEU} = 0.2403$, despite the sentences being the same, semantically. This shows an
    advantage of embeddings-based metrics over n-gram-based metrics like BLEU.

??? example "Semantically Different Texts"

    This time, let our candidate and reference texts be:

    | Candidate Text | Reference Text |
    | --- | --- |
    | The sun was setting behind the mountains. | This is a bad example |
    | She walked along the beach, feeling the sand between her toes. | This has nothing to do with the other |
    | The chef prepared a delicious meal with fresh ingredients. | Hello, world! |
    | The old oak tree stood tall in the middle of the field. | Vivaldi is a classical conductor |
    | The detective examined the clues carefully. | Wrong answer |

    This yields BERTScores of: $P_\text{BERT} = 0.8328, R_\text{BERT} = 0.8428, F_\text{BERT} = 0.8377$.
    Between different tasks, the baseline for a "good" BERTScore may vary based on different factors, like text length
    and BERT model type.

## Limitations and Biases

BERTScore, originally designed to be a replacement to the BLEU score and other n-gram similarity metrics, is a powerful
metric that closely aligns with human judgement. However, it comes with limitations.

1. BERTScore is computationally expensive. The default model (```roberta-large```) used to calculate BERTScore requires
1.4GB of weights to be stored, and requires a forward pass through the model in order to calculate the score. This may
be computationally expensive for large datasets, compared to n-gram-based metrics which are straightforward and easy to
compute. However, smaller distilled models like ```distilbert-base-uncased``` can be used instead to reduce the
computational cost, at the cost of reduced alignment with human judgement.

2. BERTScore is calculated using a black-box pretrained model. The score can not be easily explained, as the embedding
space of BERT is a dense and complex representation that is only understood by the model. Though the metric provides a
numerical score, it does not explain how or why the particular score was assigned. In contrast, n-gram-based metrics
can easily be calculated by inspection.

Limitations aside, BERTScore is still a powerful metric that can be included in NLP workflows to quantify the quality
of machine-generated texts. It has been shown to have a high correlation with human judgement in tests, and is overall
a better judge of similarity than BLEU, ROUGE, and traditional n-gram-based metrics. Furthermore, unlike traditional
metrics, BERTScore's use of embeddings allows it to factor in context, semantics, and order into its score — which
allows it to avoid the pitfalls of the traditional metrics it was designed to replace.
