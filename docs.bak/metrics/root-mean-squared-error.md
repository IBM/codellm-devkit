---
description: How to calculate and interperet RMSE for regression ML tasks
---

# Root Mean Squared Error (RMSE)

Root Mean Squared Error (RMSE) is a standard metric used to assess the performance of regression models, known for its
sensitivity to large errors. It measures the square-root of the average squared difference between the
predicted values and the actual values.

RMSE represents the square root of the [Mean Squared Error (MSE)](./mean-squared-error.md) meaning that it penalizes
large errors more heavily but is also in the same units as the output variable, making it particularly useful for
interpreting the magnitude of prediction errors and penalizing larger discrepancies more than smaller ones.
Like [MSE](./mean-squared-error.md) and [MAE](./mean-absolute-error.md) a large value is indicative of
poor performance.

!!!example
    To see and example of the Root Mean Squared Error, checkout the
    [STS Benchmark on app.kolena.com/try.](https://app.kolena.io/try/dataset/standards?datasetId=12&models=N4IglgJiBcCMDsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIZYBfOoA&models=N4IglgJiBcCMBsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIZYBfOoA&metricGroupVisibilities=N4IgbglgzhBGA2BTEAuALgJwK6IDQgFtFMIBjKVAbVEhgWXW0QF9cbo4lVMdX26ujXm3Ad63JswC6zIA)
    on Kolena's public dataset.

## Implementation Details

RMSE is calculated by first computing the mean of the squared differences between the predicted values and the actual
values, and then taking the square root of this average. This can be mathematically represented as:

$$
\sqrt{\frac{1}{N} \sum_{i=1}^{N}(x_i-y_i)^2}
$$

where $x$ is the numerical value from the actual values, and $y$ is the corresponding numerical value from the
predicted values for a total of $N$ number of predictions.

### Examples

Temperature Estimation:

<div class="grid" markdown>
| Ground Truth Temperature (&deg;C) | Predicted Temperature (&deg;C) |
| --- | --- |
| 25 | 27 |
| 35 | 30 |

$$
\begin{align}
\text{RMSE} &= \sqrt{\frac{(25 - 27)^2 + (35 - 30)^2}{2}} \\
&= \sqrt{14.5} \\
&\approx 3.81
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
\text{RMSE} &= \sqrt{\frac{(60 - 70)^2 + (40 - 20)^2}{2}} \\
&= \sqrt{250} \\
&\approx 15.8
\end{align}
$$
</div>

## Limitations and Biases

While Root Mean Squared Error (RMSE) provides a useful gauge for understanding the average magnitude of prediction
errors in the same units as the predicted value, it also emphasizes larger errors due to the squaring process. This
emphasis on larger discrepancies can sometimes overshadow the model's performance on the majority of predictions,
especially in datasets with significant outliers.

In cases where understanding the distribution of all errors (including smaller ones) is crucial, or when outliers
should not disproportionately impact the overall error metric, supplementing RMSE with other metrics like [Mean Absolute
Error (MAE)](./mean-absolute-error.md) might provide a more balanced evaluation.

Therefore, while RMSE is invaluable for highlighting large errors and providing an easily interpretable metric, it's
beneficial to consider multiple measures when evaluating the comprehensive performance of regression models.
