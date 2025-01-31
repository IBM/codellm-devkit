---
description: Measuring textual similarity with N-gram overlap
---

# BLEU

!!! info inline end "BLEU vs. Precision"

    BLEU can be thought of as an analog to [precision](precision.md) for text comparisons.

The BLEU (**B**i**L**ingual **E**valuation **U**nderstudy) score is a metric commonly used in a variety of NLP tasks
ranging from Machine Translation to Text Summarization, to evaluate the quality of candidate texts. It quantifies the
similarity between the candidate text and reference text as a score between 0 and 1 — 0 meaning there is no overlap with
the ground truth text, and 1 meaning there is a perfect overlap. As the name suggests, it was originally developed for
evaluating machine-translation models, but has since been adapted to many different tasks within NLP due to its dynamic
nature for measuring textual similarity.

## Implementation Details

### Background

The BLEU score consists of two components — the Brevity Penalty and the n-gram Overlap.

??? question "What are n-grams?"
    An n-gram is a series of `n` adjacent tokens or words in a text.
    For example, all 1-grams (or unigrams) of the sentence
    `The cat chased the squirrel` are `["the", "cat", "chased", "the", "squirrel"]`.
    The *bigrams* of the same sentence are `["the cat", "cat chased", "chased the", "the squirrel"]`.

1. The **n-gram Overlap** counts the number of 1-grams, 2-grams, 3-grams, and 4-grams of the output text that match the
1-, ..., 4-grams in the reference text — which is analogous to a precision score for the text. The 1-gram precision
ensures that the correct vocabulary is used, whereas the 4-gram precision ensures that the candidate text is coherent.
2. The **Brevity Penalty** is applied to penalize the score for generating sentences that are less in length than the
reference text. This is due to the fact that the n-gram Overlap precision tends to give disproportionately high values
to candidate texts that are very short in length, but mostly contain n-grams in the reference text.

### Definition

The BLEU score is mathematically defined as:

<!-- markdownlint-disable MD013 -->

$$\begin{align*} \text{BLEU} &= \text{Brevity Penalty} \times \text{n-gram Overlap} \\
&= \min\left(1, \, \exp\left(1 - \frac{\text{reference length}}{\text{output length}}\right)\right) \times \prod_{i=1}^{4}\text{i-gram Precision}^{\frac{1}{4}}
\end{align*}$$

<!-- markdownlint-enable MD013 -->

where the i-gram precision is calculated as:

<!-- markdownlint-disable MD013 -->

$$
p_i = \frac{\text{Clipped} \text{ count of matching i-grams in candidate text}^1}{\text{Total number of i-grams in candidate text}}
$$

<!-- markdownlint-enable MD013 -->

<div class="footnote-content">
    <p style="font-size: smaller;">
        <sup>1</sup> The <i>clipped count of matching i-grams in candidate text</i> is the minimum between the count of
        i-grams in the candidate text and the maximum count of i-grams in any of the reference texts for a given i-gram.
    </p>
</div>

### Interpretation
A known fact about BLEU scores is that they are not to be compared between different workflows and tasks. An excellent
BLEU score for one task may be subpar for another. The score only serves as a general guideline to quantify the
performance of your model — not to replace human judgement. That being said, what do these scores really mean, and how
can we decipher them?

1. **Higher is better**: Though BLEU scores may vary among tasks, one thing is for sure - higher scores are better.
Generally speaking, a commonly accepted guideline is as follows: <br>

    | Score | Qualitative Interpretation |
    | ---   | ---                        |
    | < 0.1 | Useless output             |
    | 0.1 - 0.4 | Varies in quality; May not be acceptable |
    | 0.4 - 0.6 | High quality generated text |
    | > 0.6 | Better than human quality  |

2. **Track Trends Over Time**: Rising scores signal improvements in models, while drops could hint at issues or changes
in your dataset.

3. **Combine It With Other Metrics**: BLEU primarily measures n-gram overlap, overlooking some nuances like context and
understanding. While a high BLEU score is promising, it doesn't guarantee flawless text. A complementary metric like
[BertScore](bertscore.md) may help in quantifying your model's performance from other perspectives.

## Example

| Generated | Reference |
| --- | --- |
| `Fall leaves rustled softly beneath our weary feet` | `Crisp autumn leaves rustled softly beneath our weary feet` |

??? example "Step 1: Tokenization & n-grams"
    Splitting our sentences up into 1-, ..., 4-grams, we get:

    **Generated Sentence**:

    <!-- markdownlint-disable MD013 -->

    | n | n-grams |
    | ---   | ---                        |
    | 1 | [`Fall`, `leaves`, `rustled`, `softly`, `beneath`, `our`, `weary`, `feet`]|
    | 2 | [`Fall leaves`, `leaves rustled`, `rustled softly`, `softly beneath`, `beneath our`, `our weary`, `weary feet`]|
    | 3 | [`Fall leaves rustled`, `leaves rustled softly`, `rustled softly beneath`, `softly beneath our`, `beneath our weary`, `our weary feet`] |
    | 4 | [`Fall leaves rustled softly`, `leaves rustled softly beneath`, `rustled softly beneath our`, `softly beneath our weary`, `beneath our weary feet`]  |

    <!-- markdownlint-enable MD013 -->

    **Reference Sentence**:

    <!-- markdownlint-disable MD013 -->

    | n | n-grams |
    | --- | --- |
    | 1 | [`Crisp`, `autumn`, `leaves`, `rustled`, `softly`, `beneath`, `our`, `weary`, `feet`]|
    | 2 | [`Crisp autumn`, `autumn leaves`, `leaves rustled`, `rustled softly`, `softly beneath`, `beneath our`, `our weary`, `weary feet`]|
    | 3 | [`Crisp autumn leaves`, `autumn leaves rustled`, `leaves rustled softly`, `rustled softly beneath`, `softly beneath our`, `beneath our weary`, `our weary feet`]|
    | 4 | [`Crisp autumn leaves rustled`, `autumn leaves rustled softly`, `leaves rustled softly beneath`, `rustled softly beneath our`, `softly beneath our weary`, `beneath our weary feet`]|

    <!-- markdownlint-enable MD013 -->

??? example "Step 2: Calculate n-gram Overlap"
    Next, let's calculate the clipped precision scores for each of the n-grams. Recall that the precision formula is:

    <!-- markdownlint-disable MD013 -->

    $$
    p_i = \frac{\text{Clipped} \text{ count of matching i-grams in machine-generated text}^1}{\text{Total number of i-grams in machine-generated text}}
    $$

    <!-- markdownlint-enable MD013 -->

    <center>

    | n | Clipped Precision  |
    | --- | --- |
    | 1 | 7 / 8 = 0.875 |
    | 2 | 6 / 7 = 0.857 |
    | 3 | 5 / 6 = 0.833 |
    | 4 | 4 / 5 = 0.800 |

    </center>

    So, our n-gram overlap is:

    $$
    0.875^{0.25}\cdot0.857^{0.25}\cdot0.833^{0.25}\cdot0.800^{0.25} = 0.841
    $$

??? example "Step 3: Calculate Brevity Penalty"
    We apply a brevity penalty to prevent the BLEU score from giving undeservingly high scores for short generated
    texts. Recall that our formula is:

    $$
    \min\left(1, \exp\left(1 - \frac{\text{reference length}}{\text{output length}}\right)\right)
    = \min\left(1, \exp\left(1 - \frac{\text{9}}{\text{8}}\right)\right)
    = 0.882
    $$

??? example "Step 4: Calculate BLEU"
    Combining our n-gram overlap and Brevity Penalty, our final BLEU score is:

    $$
    \text{BLEU} = \text{Brevity Penalty} \times \text{n-gram Overlap} = 0.882 \times 0.841 = 0.742
    $$

    Note that in most cases, we may take the average or max of the BLEU score with respect to multiple reference texts
    — since multiple interpretations of the same sentences can be allowed. For example, if we calculated the BLEU score
    with the reference text being `"Crisp autumn leaves rustled softly beneath our exhausted feet"`, our BLEU score
    would be 0.478 - a much lower score from a small semantic change.

## Limitations and Biases
Though BLEU is a popular metric in NLP workflows, it comes with its limitations.

1. BLEU fails to consider the semantics of texts. As seen in the example, simply changing "take the mystery out of" to
"demistify" — while the text still retains the exact same meaning — yields a much better score, going from 0.3 to 0.6.
In contrast with embeddings-based metrics like BertScore, n-gram-based metrics like BLEU only consider the words in the
candidate text, rather than the meaning. However, this is addressed by providing multiple possible reference texts when
calculating the BLEU score.
2. BLEU does not consider the order of words. Across a large candidate/reference, BLEU is unable to consider to order
and fluency of the sentences due to its short context window of 4-grams. Although BLEU can be extended to include larger
n-gram clipped precisions, it would negatively affect shorter texts. Similarly, BLEU does not consider the importance of
words in a sentence either. It weighs unimportant words like "the", "an", "too", as much as it would the nouns and verbs
in the sentence. Once again, these pitfalls are addressed by embeddings-based metrics like BertScore.

That being said, the metric still has its advantages. It is quick and easy to compute, as opposed to other metrics like
BertScore which would take significantly longer to compute and is not easy to justify. Furthermore, it is relatively
similar to human judgement, and is commonly used within NLP which allows you to easily benchmark your models with others
and identify pain points.
