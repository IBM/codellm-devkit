---
description: How to calculate the strength of monotonic correlation between 2 variables
---

# Spearman's Rank Correlation

Spearman's Rank Correlation is a statistical measure that assesses the strength and direction of the monotonic relationship
between two variables. Unlike [Pearson Correlation](pearson-correlation.md), Spearman's Rank Correlation does not assume
a linear relationship and can be used with ordinal data. The values range from -1 to +1. A value close to +1 indicates a
strong positive monotonic correlation, where as one variable increases, the other also increases. A value close to -1
signifies a strong negative monotonic correlation, meaning as one variable increases, the other decreases. A value of
0 indicates no monotonic correlation between the variables.

!!!example
    To see an example of the Spearman's Correlation, checkout the
    [STS Benchmark on app.kolena.com/try.](https://app.kolena.io/try/dataset/standards?datasetId=12&models=N4IglgJiBcCMDsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIZYBfOoA&models=N4IglgJiBcCMBsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIZYBfOoA&metricGroupVisibilities=N4IgbglgzhBGA2BTEAuALgJwK6IDQgFtFMIBjKVAbVEhgWXW0QF9cbo4lVMdX26ujXm3Ad63JswC6zIA)

## Implementation Details

The Spearman's Rank Correlation is calculated by ranking the data points and then applying
[Pearson Correlation](pearson-correlation.md) formula to the ranks. This can be mathematically represented as:

$$
\rho = \frac{\text{cov}(R(x), R(y))}{\sigma_{R(x)} \sigma_{R(y)}}
$$

where $R(x)$ denotes the ranks of $x$, and $R(y)$ denotes the ranks of $y$.

If there are no tied ranks, a simplified formula can be used:

$$
\rho = 1 - \frac{6 \sum_{i=1}^{N} d_i^2}{N(N^2 - 1)}
$$

where $d_i$ is $x_i - y_i$, and $N$ is the number of data points.

### Examples

Temperature Correlation:

<div class="grid" markdown>

| Ground Truth Temperature (&deg;C) | Predicted Temperature (&deg;C) | Ground Truth Temperature Rank | Predicted Temperature Rank | Differences $d_i$ |
| --- | --- | --- | --- | --- |
| 25  | 27  | 1  | 1  | 0   |
| 35  | 28  | 3  | 2  | 1   |
| 30  | 30  | 2  | 3  | -1  |

</div>

$$
\begin{align}
\rho &= 1 - \frac{6 \sum d_i^2}{N(N^2 - 1)} \\[1em]
&= 1 - \frac{6 (0^2 + 1^2 + (-1)^2)}{3 (3^2 - 1)} \\[1em]
&= 0.5
\end{align}
$$

## Limitations and Biases

Spearman's rank correlation coefficient is effective for measuring the strength and direction of monotonic
relationships between variables, and it does not assume linearity. However, it is still sensitive to outliers, which
can affect the ranks and distort the results. Additionally, Spearman's correlation requires the data to be ranked,
which may not be suitable for all types of data. In datasets with significant outliers or non-monotonic relationships,
Spearman's rank correlation may not provide an accurate representation.
