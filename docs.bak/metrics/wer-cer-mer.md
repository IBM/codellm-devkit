---
description: Measuring textual similarity with substitution, deletion, and insertion errors
---

# WER, CER, and MER

Word Error Rate (WER), Character Error Rate (CER), and Match Error Rate (MER) are essential metrics used in the
evaluation of speech recognition and natural language processing systems. From a high level, they each quantify
the similarity between reference and candidate texts, with zero being a perfect score. While word and character
error rate can be infinitely high, match error rate is always between 0 and 1. Each of these metrics
have their nuances that reveal different errors within texts.

!!!example
    To see and example of WER, CER and MER, checkout the
    [LibriSpeech on app.kolena.com/try.](https://app.kolena.io/try/dataset/standards?datasetId=22&models=N4IglgJiBcBsAcAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYBGAX3qA&models=N4IglgJiBcBsDsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYBGAX3qA&models=N4IglgJiBcBssBoQGMD2A7AZmA5gVwCcBDAFzAxlEhgEYBfOoA&metricGroupVisibilities=N4IgbglgzhBGA2BTEAuALgJwK6IDQgFtFMIBjKVAbVEhgWXW0QF9cbo4lVMdX26ujXgF1mQA)

## Substitutions, Deletions, and Insertions

The building blocks of each metric include substitution, deletion, and insertion errors. These errors reveal different
failures in candidate texts, and are aggregated to calculate the word, character, and match error rate.

??? example "Substitution Errors"
    Substitutions occur when a candidate text contains a word or sequence of words that is different from the
    corresponding word or sequence of words in the reference text. Substitutions can be counted on a word,
    character, or sentence level depending on the application.

    <b> Example: </b> <br>
    Reference: `Amidst the emerald meadow, butterflies whispered secrets in the breeze.` <br>
    Candidate: `Amidst the emerald shadow, butterflies whistled secrets on the breeze.` <br>

    <b>Word-level Substitutions</b>: <br>
    <code>Amidst the emerald <span class="mg-color-substitution">
    <big>shadow</big></span>, butterflies <span class="mg-color-substitution">
    <big>whistled</big></span> secrets <span class="mg-color-substitution">
    <big>on</big></span> the breeze.</code> <br>
    <b>Character-level Substitutions</b>: <br>
    <code>Amidst the emerald <span class="mg-color-substitution">
    <big>sh</big></span>adow, butterflies <span class="mg-color-substitution">
    <big>whistl</big></span>ed secrets <span class="mg-color-substitution">
    <big>o</big></span>n the breeze.</code> <br>

    In the above example, there are 3 word-level substitutions and 9 character-level substitutions.

??? example "Deletion Errors"
    Deletions occur when a candidate text is missing a word or sequence of words from the reference text.

    <b> Example: </b> <br>
    Reference: `Amidst the emerald meadow, butterflies whispered secrets in the breeze.` <br>
    Candidate: `Amidst the emerald meadow, butterflies whispered.` <br>

    <b>Word-level Deletions</b>: <br>
    <code>Amidst the emerald meadow, butterflies whispered <span class="mg-color-deletion">
    <big>secrets in the breeze</big></span>.</code> <br>
    <b>Character-level Deletions</b>: <br>
    <code>Amidst the emerald meadow, butterflies whispered <span class="mg-color-deletion">
    <big>secrets in the breeze</big></span>.</code> <br>

    In the above example, there are 4 word-level deletions and 18 character-level deletions.

??? example "Insertion Errors"
    Insertions occur when a candidate text contains an extra word or sequence of
    words that is not present in the reference text.

    <b> Example: </b> <br>
    Reference: `Amidst the emerald meadow, butterflies whispered secrets in the breeze.` <br>
    Candidate: `Amidst the emerald meadow, butterflies whispered ethereal secrets in the breeze.` <br>

    <b>Word-level Insertions</b>: <br>
    <code>Amidst the emerald meadow, butterflies whispered <span class="mg-color-insertion">
    <big>ethereal</big></span> secrets in the breeze.</code> <br>
    <b>Character-level Insertions</b>: <br>
    <code>Amidst the emerald meadow, butterflies whispered <span class="mg-color-insertion">
    <big>ethereal</big></span> secrets in the breeze.</code> <br>

    In the above example, there is 1 word-level insertion and 8 character-level insertions.

## Word Error Rate

Word Error Rate is a fundamental metric that measures the accuracy of a candidate text by considering three types
of errors — [substitutions, deletions, and insertions](#substitutions-deletions-and-insertions). Word-level errors
surface mispredicted words, and it can be useful to visualize common word-level failures to flesh out weaknesses
in a model.

Formally, it is defined as the rate of word-level errors in a candidate text.

$$
\text{WER} = \frac{\text{Substitutions} + \text{Deletions} + \text{Insertions}}{\text{# of Words in Reference}}
$$

### Example

Let's calculate the word error rate between the following reference and candidate texts:

| <b>Reference</b> | <b>Candidate</b> |
|-|-|
|  `The bard sang ancient melodies of nature, transforming tranquil meadows into sonnets for enhanced soulful grace.` | `The poetic bard echoed ancient melodies, transcending meadows into sonnets for enhanced soulful grace.` |

??? example "Step 1. Count Errors"
    Highlighting the <span class="mg-color-substitution"><big>substitution</big></span>,
    <span class="mg-color-deletion"><big>deletion</big></span>,
    and <span class="mg-color-insertion"><big>insertion</big></span> errors,
    we can count each type of error:

    <code>
    The <span class="mg-color-insertion">
    <big>poetic</big></span> bard <span class="mg-color-substitution">
    <big>echoed</big></span> ancient melodies <span class="mg-color-deletion">
    <big>of nature</big></span>, <span class="mg-color-substitution">
    <big>transcending</big></span> <span class="mg-color-deletion">
    <big>tranquil</big></span> meadows into sonnets for enhanced soulful grace.
    </code>

    In our candidate text, we have 2 substitutions, 3 deletions, and 1 insertion.

??? example "Step 2. Calculate WER"
    With each errors counted, we can calculate our WER. Using the formula,

    <!-- markdownlint-disable MD013 -->

    $$
    \begin{align*}
    \text{WER} &= \frac{\text{Substitutions} + \text{Deletions} + \text{Insertions}}{\text{# of Words in Reference}} \\
               &= \frac{2 + 3 + 1}{16} \\
               &= \frac{6}{16} \\
               &= 0.375
    \end{align*}
    $$

    <!-- markdownlint-enable MD013 -->


    we arrive at a WER of 0.375 for our candidate text.

It is important to note that WER's range is not bounded above by 1. If we had a reference of "`hello`" and
candidate of "`bye bye`", assuming we calculate the error using only substitutions, our WER would be 2.0 since
we have 2 errors in the candidate and 1 word in the reference. Generally speaking, we want our WER to be as close
to 0 as possible.

## Character Error Rate

Character Error Rate is another metric that measures the accuracy of a candidate text through [substitutions,
deletions, and insertions](#substitutions-deletions-and-insertions). Unlike word-level errors, character-level
errors are useful in surfacing mispronunciations and erroneous phonemes. CER is defined as the rate of
character-level errors in a candidate text.

$$
\text{CER} = \frac{\text{Substitutions} + \text{Deletions} + \text{Insertions}}{\text{# of Characters in Reference}}
$$

### Example

Let's calculate the character error rate using the same reference and candidate texts as the previous example:

| <b>Reference</b> | <b>Candidate</b> |
|-|-|
|  `The bard sang ancient melodies of nature, transforming tranquil meadows into sonnets for enhanced soulful grace.` | `The poetic bard echoed ancient melodies, transcending meadows into enhanced sonnets for soulful grace.` |

??? example "Step 1. Count Errors"
    Highlighting the <span class="mg-color-substitution"><big>substitution</big></span>,
    <span class="mg-color-deletion"><big>deletion</big></span>,
    and <span class="mg-color-insertion"><big>insertion</big></span> errors,
    we can count each type of error:

    <code>
    The <span class="mg-color-insertion">
    <big>poetic</big></span> bard <span class="mg-color-substitution">
    <big>echoed</big></span> ancient melodies <span class="mg-color-deletion">
    <big>of</big></span> <span class="mg-color-deletion">
    <big>nature</big></span>, trans<span class="mg-color-substitution">
    <big>cending</big></span> <span class="mg-color-deletion">
    <big>tranquil</big></span> meadows into sonnets for enhanced soulful grace.
    </code>

    In our candidate text, we have 13 substitutions, 16 deletions, and 6 insertions.

??? example "Step 2. Calculate CER"
    With each errors counted, we can calculate our CER. Using the formula,

    <!-- markdownlint-disable MD013 -->

    $$
    \begin{align*}
    \text{CER} &= \frac{\text{Substitutions} + \text{Deletions} + \text{Insertions}}{\text{# of Characters in Reference}} \\
               &= \frac{13 + 16 + 6}{110} \\
               &= \frac{35}{110} \\
               &= 0.318
    \end{align*}
    $$

    <!-- markdownlint-enable MD013 -->


    we arrive at a CER of 0.318 for our candidate text. Note that the CER is lower than the WER calculated
    in the last step. Although the errors are similar between the two calculations, the character-level
    substitution only replaces `trans`, whereas the word-level substitution replaces the entire `transforming`
    — showing that our model could be weak at recognizing the specific phonemes coming after trans-. However,
    this would be hard to confirm without more data.

It is valuable to use CER alongside WER in speech recognition and NLP tasks, as each metric can surface different
types of errors. A model with a high WER but low CER can indicate that the model is mainly mispredicting specific
phonemes rather than entire words, whereas a balanced WER and CER can indicate poor ability to make predictions
at the word level.

## Match Error Rate

While WER and CER focus on errors, Match Error Rate takes a slightly different approach by placing more emphasis
on correct matches. Similar to WER, it is calculated using word-level [substitutions, deletions, and insertions](#substitutions-deletions-and-insertions).

<!-- markdownlint-disable MD013 -->

$$
\text{MER} = \frac{\text{Substitutions} + \text{Deletions} + \text{Insertions}}{\text{Substitutions} + \text{Deletions} + \text{Insertions} + \text{# of Correct Matches}}
$$

<!-- markdownlint-enable MD013 -->

### Example

Let's calculate the match error rate using the same reference and candidate texts as the previous examples:

| <b>Reference</b> | <b>Candidate</b> |
|-|-|
|  `The bard sang ancient melodies of nature, transforming tranquil meadows into sonnets for enhanced soulful grace.` | `The poetic bard echoed ancient melodies, transcending meadows into enhanced sonnets for soulful grace.` |

??? example "Step 1. Count Errors"
    Highlighting the <span class="mg-color-substitution"><big>substitution</big></span>,
    <span class="mg-color-deletion"><big>deletion</big></span>,
    and <span class="mg-color-insertion"><big>insertion</big></span> errors,
    we can count each type of error:

    <code>
    The <span class="mg-color-insertion"><big>poetic</big></span> bard <span class="mg-color-substitution">
    <big>echoed</big></span> ancient melodies <span class="mg-color-deletion">
    <big>of</big></span> <span class="mg-color-deletion">
    <big>nature</big></span>, <span class="mg-color-substitution">
    <big>transcending</big></span> <span class="mg-color-deletion">
    <big>tranquil</big></span> meadows into sonnets for enhanced soulful grace.
    </code>

    In our candidate text, we have 2 substitutions, 3 deletions, and 1 insertion.

??? example "Step 2. Calculate MER"
    With each errors counted, we can calculate our MER. Using the formula,

    <!-- markdownlint-disable MD013 -->

    $$
    \begin{align*}
    \text{MER} &= \frac{\text{Substitutions} + \text{Deletions} + \text{Insertions}}{\text{Substitutions} + \text{Deletions} + \text{Insertions} + \text{# of Correct Matches}} \\
               &= \frac{2 + 3 + 1}{2 + 3 + 1 + 11} \\
               &= \frac{6}{17} \\
               &= 0.353
    \end{align*}
    $$

    <!-- markdownlint-enable MD013 -->


    we arrive at a MER of 0.353 for our candidate text. This is roughly in line with what we had for CER and WER.

In general, all three metrics are similar, yet reveal slightly different hidden errors within the candidate text.

## Limitations and Biases

As seen in its formula, WER, CER, and MER only accept perfect matches between words, while placing no consideration
on alternate spellings. For example, a candidate text could be penalized if it had the word "gray" while the
reference text had the word "grey". Though the two spellings are perfectly acceptable and do not change a
sentence's meaning, these metrics fail to consider this. Although these types of false errors can be mitigated
through a rule-based error calculation, it adds extra complexity and provides no guarantee of mitigating all
false errors.
