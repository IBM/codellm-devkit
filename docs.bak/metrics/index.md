---
icon: kolena/metrics-glossary-16
hide:
  - toc
description: A glossary for machine learning metrics
---

# :kolena-metrics-glossary-20: Metrics Glossary

This section contains guides for different metrics used to measure model performance.

Each ML use case requires different metrics. Using the right metrics is critical for understanding and meaningfully
comparing model performance. In each metrics guide, you can learn about the metric with examples, its limitations and
biases, and its intended uses.

<div class="grid cards" markdown>

- [Accuracy](accuracy.md)

    ---

    Accuracy measures how well a model predicts correctly. It's a good metric for assessing model performance in simple
    cases with balanced data.

- [Average Precision (AP)](average-precision.md)

    ---

    Average precision summarizes a precision-recall (PR) curve into a single threshold-independent value
    representing model's performance across all thresholds.

- [Averaging Methods: Macro, Micro, Weighted](averaging-methods.md)

    ---

    Different averaging methods for aggregating metrics for **multiclass** workflows, such as classification and
    object detection.

- [Coefficient of Determination (R²)](coefficient-of-determination.md)

    ---

    R² provides a scale that is generally from 0 to 1, where higher values indicate a better fit and imply that
    the model can better explain the variation of the model inferences.

- [Cohen's Kappa](cohens-kappa.md)

    ---

    Cohen's Kappa measures the agreement between model inferences and ground truths. It's a good performance metric
    that considers class imbalance.

- [Confusion Matrix](confusion-matrix.md)

    ---

    Confusion matrix is a structured plot describing classification model performance as a table that highlights counts
    of objects with predicted classes (columns) against the actual classes (rows), indicating how confused a model is.

- [F<sub>1</sub>-score](f1-score.md)

    ---

    F<sub>1</sub>-score is a metric that combines two competing metrics, [precision](precision.md) and
    [recall](recall.md) with an equal weight. It symmetrically represents both precision and recall as one metric.

- [False Positive Rate (FPR)](fpr.md)

    ---

    False positive rate (FPR) measures the proportion of negative ground truths that a model incorrectly predicts as
    positive, ranging from 0 to 1. It is useful when the objective is to measure and reduce false positive inferences.

- [Mean Absolute Error (MAE)](mean-absolute-error.md)

    ---

    MAE measures the average magnitude of errors in predictions, without considering their direction. It represents
    the mean of the absolute differences between predicted and actual values across a dataset, treating each
    discrepancy equally.

- [Mean Squared Error (MSE)](mean-squared-error.md)

    ---

    MSE measures the average squared difference between the predicted values and the actual values,
    emphasizing larger errors more significantly than smaller ones. It is mainly different from
    [MAE](mean-absolute-error.md) in that larger errors are penalized more heavily.

- [Pearson Correlation](pearson-correlation.md)

    ---

    Pearson Correlation is a statistical measure that evaluates the strength and
    direction of the linear relationship between two continuous variables. The values range from -1 to +1.

- [Precision](precision.md)

    ---

    Precision measures the proportion of positive inferences from a model that are correct. It is useful when the
    objective is to measure and reduce false positive inferences.

- [Precision-Recall (PR) Curve](pr-curve.md)

    ---

    Precision-recall curve is a plot that gauges machine learning model performance by using [precision](precision.md)
    and [recall](recall.md). It is built with precision on the y-axis and recall on the x-axis computed across many
    thresholds.

- [Recall (TPR, Sensitivity)](recall.md)

    ---

    Recall, also known as true positive rate (TPR) and sensitivity, measures the proportion of all positive ground
    truths that a model correctly predicts. It is useful when the objective is to measure and reduce false negative
    ground truths, i.e. model misses.

- [Receiver Operating Characteristic (ROC) Curve](roc-curve.md)

    ---

    A receiver operating characteristic (ROC) curve is a plot that is used to evaluate the performance of binary
    classification models by using the [true positive rate (TPR)](./recall.md) and the
    [false positive rate (FPR)](./fpr.md).

- [Root Mean Squared Error (RMSE)](root-mean-squared-error.md)

    ---

    RMSE measures the square root of the average squared difference between the predicted values and the
    actual values, emphasizing larger errors more significantly than smaller ones while maintaing the same
    unit as the predicted and actual values which is what differentiates it from [MSE](mean-squared-error.md).

- [Spearman's Rank Correlation](spearman-correlation.md)

    ---

    Spearman's Rank Correlation coefficient is a statistical measure that assesses the strength and
    direction of the monotonic relationship between two variables. Unlike Pearson Correlation, Spearman's Rank Correlation
    does not assume a linear relationship and can be used with ordinal data. The values range from -1 to +1.

- [Specificity (TNR)](specificity.md)

    ---

    Specificity, also known as true negative rate (TNR), measures the proportion of negative ground truths that a
    model correctly predicts, ranging from 0 to 1. It is useful when the objective is to measure the model's ability to
    correctly identify the negative class instances.

- [TP / FP / FN / TN](tp-fp-fn-tn.md)

    ---

    The counts of TP, FP, FN and TN ground truths and inferences are essential for summarizing model performance. They
    are the building blocks of many other metrics, including [accuracy](accuracy.md), [precision](precision.md),
    and [recall](recall.md).

</div>

## Computer Vision

<div class="grid cards" markdown>

- [Geometry Matching](geometry-matching.md)

    ---

    Geometry matching is the process of matching inferences to ground truths for computer vision workflows with a
    localization component. It is a core building block for metrics such as [TP, FP, and FN](tp-fp-fn-tn.md), and any
    metrics built on top of these, like [precision](precision.md), [recall](recall.md), and
    [F<sub>1</sub>-score](f1-score.md).

- [Intersection over Union (IoU)](iou.md)

    ---

    IoU measures overlap between two geometries, segmentation masks, sets of labels, or time-series snippets.
    Also known as Jaccard index in classification workflow.

</div>

## Large Language Models

<div class="grid cards" markdown>

- [Consistency Score](consistency-score.md)

    ---

    The consistency score is a sampling-based approach that computes a score by aggregating across `n` sample responses
    obtained by using the same prompt `n` times. The model is considered hallucinating if there exists sample responses
    that contradict one another or are not consistent.

- [Contradiction Score](contradiction-score.md)

    ---

    NLI classification is an insightful tool to measure if candidate texts and reference texts are contradictory or
    consistent, providing extra detail towards hallucination detection.

- [HHEM Score](HHEM-score.md)

    ---

    [Hughes Hallucination Evaluation Model (HHEM)](https://huggingface.co/vectara/hallucination_evaluation_model) is an
    open-source model that can be used to compute scores for hallucination detection.

- [Prompt-based Hallucination Metric](prompt-based-hallucination-metric.md)

    ---

    Large Language Model (LLM) Prompt-based Metric involves using an LLM and various prompt engineering techniques
    to perform evaluations. These methods include chain-of-thought, self-consistency, and others to determine whether or
    not an inference text contains a hallucination.

</div>

## Natural Language Processing

<div class="grid cards" markdown>

- [BERTScore](bertscore.md)

    ---

    BERTScore is a metric used in NLP workflows to measure textual similarity between candidate texts and reference
    texts.

- [BLEU](bleu.md)

    ---

    BLEU is a metric commonly used in a variety of NLP workflows to evaluate the quality of candidate texts. BLEU can be
    thought of as an analog to [precision](precision.md) for text comparisons.

- [Diarization Error Rate](diarization-error-rate.md)

    ---

    Diarization Error Rate (DER) is an important metric used to evaluate speaker diarization systems. It quantifies the
    overall performance of a speaker diarization system by measuring the ratio of the duration of errors to the total
    ground truth speech duration.

- [METEOR](meteor.md)

    ---

    METEOR is a widely recognized and vital metric used in NLP. It is used to measure the quality of candidate texts
    against reference texts. Though it is an n-gram based metric, it goes beyond traditional methods by factoring in
    elements such as precision, recall, and order to provide a comprehensive measure of text quality.

- [Perplexity](perplexity.md)

    ---

    Perplexity is a metric commonly used in natural language processing to evaluate the quality of language models,
    particularly in the context of text generation. Unlike metrics such as BLEU or BERT, perplexity doesn't directly
    measure the quality of generated text by comparing it with reference texts. Instead, perplexity assesses the
    "confidence" or "surprise" of a language model in predicting the next word in a sequence of words.

- [ROUGE-N](rouge-n.md)

    ---

    ROUGE-N, a metric within the broader ROUGE metric collection, is a vital metric in the field of NLP. It assesses
    the quality of a candidate text by measuring the overlap of n-grams between the candidate text and reference texts.
    ROUGE-N can be thought of as an analog to [recall](recall.md) for text comparisons.

- [Word, Character, and Match Error Rate](wer-cer-mer.md)

    ---

    Word Error Rate (WER), Character Error Rate (CER), and Match Error Rate (MER) are essential metrics used in the
    evaluation of speech recognition and natural language processing systems. From a high level, they each quantify
    the similarity between reference and candidate texts, with zero being a perfect score.

</div>

## Kolena Insights

<div class="grid cards" markdown>

- [Difficulty Score](difficulty-score.md)

    ---

    Difficulty scores indicate which datapoints models commonly struggle with based on custom Quality Standards.
    The greater the difficulty score, the harder it is for models to be certain and performant.

- [Statistical Significance](statistical-significance.md)

    ---

    Statistical significance represents the amount of tolerance in your evaluation results based on a chosen confidence
    level and the size of your test case. Significance is derived by estimating the margin of error, which ranges from
    0% to 100%. The larger the margin of error, the less confidence one should have that a result would reflect the
    result of a fully representative test set.

</div>
