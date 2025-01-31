---
description: How to calculate the strength of linear correlation between 2 variables
---

# Pearson Correlation

Pearson Correlation is a statistical measure that evaluates the strength and direction of the linear
relationship between two continuous variables. The values range from -1 to +1. A value close to +1 indicates a strong
positive linear correlation, where as one variable increases, the other also increases proportionally. A value close
to -1 signifies a strong negative linear correlation, meaning as one variable increases, the other decreases
proportionally. A value of 0 indicates no linear correlation between the variables.

!!!example
    To see an example of the Pearson Correlation, checkout the
    [STS Benchmark on app.kolena.com/try.](https://app.kolena.io/try/dataset/standards?datasetId=12&models=N4IglgJiBcCMDsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIZYBfOoA&models=N4IglgJiBcCMBsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIZYBfOoA&metricGroupVisibilities=N4IgbglgzhBGA2BTEAuALgJwK6IDQgFtFMIBjKVAbVEhgWXW0QF9cbo4lVMdX26ujXm3Ad63JswC6zIA)

## Implementation Details

The correlation coefficient is calculated by dividing the covariance of $x$ and $y$ by their individual standard
deviations. This can be mathematically represented as:

$$
r = \frac{\text{cov}(x, y)}{\sigma_x \sigma_y} = \frac{\sum_{i=1}^{N} (x_i - \bar{x})(y_i - \bar{y})}{\sqrt{\sum_{i=1}^{N}
(x_i - \bar{x})^2 \sum_{i=1}^{N} (y_i - \bar{y})^2}}
$$

where $x$ and $y$ are the collection of data points.

### Examples

Temperature Correlation:

<div class="grid" markdown>

| Ground Truth Temperature (&deg;C) | Predicted Temperature (&deg;C) |
| --- | --- |
| 25  | 27  |
| 35  | 28  |
| 30  | 30  |

</div>

$$
\begin{align}
\bar{x} &= \frac{25 + 35 + 30}{3} = 30 \\[1em]
\bar{y} &= \frac{30 + 28 + 27}{3} = 28.33 \\[1em]
r &= \frac{\sum_{i=1}^{4} (x_i - \bar{x})(y_i - \bar{y})}{\sqrt{\sum_{i=1}^{3} (x_i - \bar{x})^2 \sum_{i=1}^{3} (y_i -
\bar{y})^2}} \\[1em]
&= \frac{(25 - 30)(27 - 28.33) + (35 - 30)(28 - 28.33) + (30 - 30)(30 - 28.33)}{\sqrt{[(25 - 30)^2 +
(35 - 30)^2 + (30 - 30)^2] [(30 - 28.33)^2 + (28 - 28.33)^2 + (27 - 28.33)^2 ]}} \\[1em]
&\approx 0.33
\end{align}
$$

## Limitations and Biases

Pearson Correlation is useful for measuring the strength and direction of linear relationships between
variables. However, it assumes linearity and is sensitive to outliers, which can distort the results. In datasets with
non-linear relationships or significant outliers, Pearson Correlation may not provide an accurate
representation. For non-linear relationships, [Spearman's Rank Correlation](spearman-correlation.md) is a better choice.
