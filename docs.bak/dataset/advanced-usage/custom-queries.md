---
icon: kolena/developer-16
---

# :kolena-developer-20: Custom Queries and Fields

Custom queries allow you to filter and sort your data flexibly and find datapoints of interest faster.
The custom query function can be accessed from the filter and sort components.

<figure markdown>
![Access Custom Queries](../../assets/images/custom-queries-dark.gif#only-dark)
![Access Custom Queries](../../assets/images/custom-queries-light.gif#only-light)
<figcaption>Access Custom Queries</figcaption>
</figure>

Custom fields allow you configure new fields based on existing fields using common operations.

<figure markdown>
![Access Custom Fields](../../assets/images/custom-fields-dark.gif#only-dark)
![Access Custom Fields](../../assets/images/custom-fields-light.gif#only-light)
<figcaption>Access Custom Fields</figcaption>
</figure>

You can access datapoint fields by typing `@datapoint.` or result fields via `@result.`.

## Details

This table summarized the the available operations.

| **Category**             | **Operators** | **Example**                                            | **Description**                                                                                                                                                                                                                                                                                  |
|--------------------------|---------------|--------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Logical Operators**    | `and`         | `@datapoint.a > 2 and @datapoint.b < 3`                | Logical AND                                                                                                                                                                                                                                                                                      |
|                          | `or`          | `@datapoint.a <= 4 or @datapoint.b >= 5`               | Logical OR                                                                                                                                                                                                                                                                                       |
| **Relational Operators** | `==`          | `@datapoint.a == 10`                                   | Equal to                                                                                                                                                                                                                                                                                         |
|                          | `!=`          | `@datapoint.a != 5`                                    | Not equal to                                                                                                                                                                                                                                                                                     |
|                          | `>`           | `@datapoint.a > 20`                                    | Greater than                                                                                                                                                                                                                                                                                     |
|                          | `>=`          | `@datapoint.a >= 15`                                   | Greater than or equal to                                                                                                                                                                                                                                                                         |
|                          | `<`           | `@datapoint.a < 30`                                    | Less than                                                                                                                                                                                                                                                                                        |
|                          | `<=`          | `@datapoint.a <= 25`                                   | Less than or equal to                                                                                                                                                                                                                                                                            |
| **Arithmetic Operators** | `+`           | `@datapoint.a + 5`                                     | Addition                                                                                                                                                                                                                                                                                         |
|                          | `-`           | `@datapoint.a - 3`                                     | Subtraction                                                                                                                                                                                                                                                                                      |
|                          | `*`           | `@datapoint.a * 2`                                     | Multiplication                                                                                                                                                                                                                                                                                   |
|                          | `/`           | `@datapoint.a / 4`                                     | Division                                                                                                                                                                                                                                                                                         |
| **Power Operator**       | `^`           | `@datapoint.a ^ 2`                                     | Power                                                                                                                                                                                                                                                                                            |
| **Numeric Functions**    | `abs`         | `abs(@datapoint.a - 10)`                               | Returns the absolute value.                                                                                                                                                                                                                                                                      |
|                          | `floor`       | `floor(@datapoint.a)`                                  | Returns the value rounded to the nearest equal or smaller integer.                                                                                                                                                                                                                               |
|                          | `round`       | `round(@datapoint.a)`                                  | Returns the rounded value.                                                                                                                                                                                                                                                                       |
|                          | `sqrt`        | `sqrt(@datapoint.a)`                                   | Returns the square root value.                                                                                                                                                                                                                                                                   |
| **String Functions**     | `lower`       | `lower(@datapoint.a)`                                  | Returns the string value with all characters converted to lowercase.                                                                                                                                                                                                                             |
|                          | `upper`       | `upper(@datapoint.a)`                                  | Returns the string value with all characters converted to uppercase.                                                                                                                                                                                                                             |
|                          | `ltrim`       | `ltrim(@datapoint.a [, <characters>])`                 | Returns the string value with left-side characters matching `<characters>` removed.<br>If `<characters>` is unspecified, it defaults to `' '`, a blank space character.                                                                                                                          |
|                          | `replace`     | `replace(@datapoint.a, <pattern> [, <replacement>])`   | Returns the string value with instances of the substring `<pattern>` converted to `<replacement>`.<br>If `<replacement>` is unspecified, all instances of `<pattern>` are deleted instead.                                                                                                       |
|                          | `substr`      | `substr(@datapoint.a, <start> [, <length>])`           | Returns the substring within the value starting with the `<start>` index and with `<length>` characters.<br>The start position is 1-indexed, not 0-indexed. For example, `substr('abc', 1, 1)` returns `a`, not `b`.<br>If `<length>` is unspecified, all characters until the end are returned. |
| **Array Functions**      | `array_size`  | `array_size(@datapoint.a[])`                           | Returns the number of objects in the array.                                                                                                                                                                                                                                                      |
|                          | `filter`      | `filter(@datapoint.a, val -> val.area >= 400)`         | Returns an array with objects that have an area of more than 400.                                                                                                                                                                                                                                |

!!! Example
    **Combining Logical and Relational Operators**

    ```dsl
    @datapoint.a > 10 and @datapoint.b < 20
    ```
    ```dsl
    abs(@resultA.recall - @resultB.recall) >= 0.2
    ```

    **Using Arithmetic and Power Operators**

    ```dsl
    @datapoint.a * @datapoint.b + @datapoint.c ^ 2
    ```

    **Using Functions in Expressions**

    ```dsl
    abs(@datapoint.a - 10)
    ```
    ```dsl
    sqrt(@datapoint.b + @datapoint.c)
    ```

    **Using String Functions**

    ```dsl
    replace(lower(@datapoint.locator), "s3", "gs")
    ```
    ```dsl
    substr(ltrim(@datapoint.text), 5, 3)
    ```

    **Using array filters and size**

    This function returns the number of objects that have an area of more than 400
    ```dsl
    array_size(filter(@datapoint.b[], val -> val.area >= 400))
    ```
