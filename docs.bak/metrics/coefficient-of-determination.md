---
description: How to calculate and interperet R² for regression ML tasks
---

# Coefficient of Determination (R²)

The Coefficient of Determination, commonly denoted as R², is a statistical measure used to assess the goodness of fit
of a regression model. It represents the proportion of the variance in the model inferences that is
explainable by the model itself.

R² provides a scale that is **generally** from 0 to 1, where higher values indicate a better fit and imply that
the model can better explain the variation of the inferences. It is particularly useful for
comparing the explanatory power of different models and their overall fit. Negative values are possible and indicate
that the mean of the data itself is a better fit to the data than the regressor/model itself.

## Implementation Details

R² is calculated as the proportion of the total variation of outcomes explained by the model. Mathematically, it can
be represented as:

$$
R² = 1 - \frac{\sum_{i=1}^{N}(y_i - \hat{y}_i)^2}{\sum_{i=1}^{N}(y_i - \bar{y})^2}
$$

where $y_i$ is the actual value, $\hat{y}_i$ is the predicted value from the model, $\bar{y}$ is the mean of the
actual values, and $N$ is the total number of observations.

### Examples

Temperature Estimation:

<div class="grid" markdown>
| Ground Truth Temperature (°C) | Predicted Temperature (°C) |
| ----------------------------- | -------------------------- |
| 25                            | 27                         |
| 35                            | 30                         |

$$
\begin{align}
R² &= 1 - \frac{(25 - 27)^2 + (35 - 30)^2}{(25 - 30)^2 + (35 - 30)^2} \\
&= 0.42
\end{align}
$$
</div>

Age Estimation:

<div class="grid" markdown>
| Ground Truth Age (Years) | Predicted Age (Years) |
| ------------------------ | --------------------- |
| 60                       | 70                    |
| 40                       | 20                    |

$$
\begin{align}
R² &= 1 - \frac{(60 - 70)^2 + (40 - 20)^2}{(60 - 50)^2 + (40 - 50)^2} \\
&= -1.5
\end{align}
$$
</div>

## Limitations and Biases

When assessing regression models on unseen data, R² quantifies how well model predictions align with observed outcomes
indicating the variance explained by the model. However, its utility is nuanced:

- R² might be misleading if the unseen data's distribution diverges from the training data,
hinting at potential data characteristic shifts in addition to model overfitting.
- It might not capture the full spectrum of model accuracy in complex, non-linear scenarios, where direct error
metrics offer clearer insights.
- High R² values do not imply that changes in predictors causally affect the outcome variable; R² measures
correlation strength, not causation.

When evaluating the model's performance under varying conditions, it is important to consider the impact of
these conditions on the mean of the ground truth, as this influences the R² value.
This is why if you were to stratify by age in an age estimation problem, you may even get negative values
depending on the granularity of the strata/test-cases.

Error metrics like [Mean Absolute Error (MAE)](./mean-absolute-error.md) and
[Mean Squared Error (MSE)](./mean-squared-error.md) enrich model evaluation. MAE provides the
average prediction error magnitude, while MSE emphasizes larger errors, offering a balance to R²'s variance explanation by
underscoring prediction accuracy and error severity.
