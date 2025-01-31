---
description: How to calculate and interpret MSE for regression ML tasks
---

# Mean Squared Error (MSE)

Mean Squared Error (MSE) is a widely used metric for evaluating the performance of regression models,
popular for its ability to accentuate larger errors. It measures the average squared difference between the
predicted values and the actual values, emphasizing larger errors more significantly than smaller ones.

MSE represents the mean of the squared discrepancies between predicted and actual values across a dataset,
penalizing larger errors more heavily. A large value is indicative of poor performance, however the metric is not in
the same scale/unit as the predictions and ground truths.

## Implementation Details

MSE is calculated by taking the average of the squared differences between the predicted values and the actual values.
This can be mathematically represented as:

$$
\frac{1}{N} \sum_{i=1}^{N}(x_i-y_i)^2
$$

where $x$ is the numerical value from the actual values, and $y$ is the corresponding numerical value from
the predicted values for a total of $N$ number of predictions.

### Examples

Temperature Estimation:

<div class="grid" markdown>
| Ground Truth Temperature (&deg;C) | Predicted Temperature (&deg;C) |
| --- | --- |
| 25 | 27 |
| 35 | 30 |

$$
\begin{align}
\text{MSE} &= \frac{(25 - 27)^2 + (35 - 30)^2}{2} \\
&= 14.5
\end{align}
$$
</div>

Age Estimation:

<div class="grid" markdown>
| Ground Truth Age (Years) | Predicted Age (Years) |
| --- | --- |
| 60 | 70 |
| 40 | 20 |

$$
\begin{align}
\text{MSE} &= \frac{(60 - 70)^2 + (40 - 20)^2}{2} \\
&= 250
\end{align}
$$
</div>

## Limitations and Biases

While Mean Squared Error (MSE) offers a clear metric for evaluating regression model accuracy by heavily penalizing
larger errors, this same feature can also be seen as a limitation. MSE can disproportionately represent the effect of
outliers or extreme errors on the overall model performance, potentially leading to a skewed perception of a model's
predictive ability.

In contexts where outliers are significant or the distribution of errors is important, relying solely on MSE may not
provide a fully accurate evaluation. It is crucial to supplement MSE with other metrics, such as [Mean Absolute Error
(MAE)](./mean-absolute-error.md) or Root Mean Squared Error (RMSE), to gain a more nuanced understanding of model
performance.

Therefore, while MSE is valuable for identifying and correcting large prediction errors, it's advisable to consider a
range of metrics for a comprehensive assessment of regression models.
