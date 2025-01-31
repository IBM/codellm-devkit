---
icon: kolena/media-16
---

# :kolena-media-20: Automatically Extract Image Properties

This guide outlines how to configure the extraction of properties from Images on Kolena.

## Configuring Image Property Extraction

??? "1. Navigate to Dataset Details"
    Scroll down to the "Details" page of your dataset.

    <figure markdown>
    ![Navigating to Configuration](../assets/images/navigate-to-image-extraction-config-dark.gif#only-dark)
    ![Navigating to Configuration](../assets/images/navigate-to-image-extraction-config-light.gif#only-light)
    <figcaption>Navigating to Image Property Configuration</figcaption>
    </figure>

??? "2. Select Image Fields and Properties"
    Identify and select the image field(s) from your dataset that you want to analyze.
    Also select the properties of the field(s) you wish to extract.

    In the example below we extract properties from the `locator` field.

    <figure markdown>
    ![Select Properties of Image Fields](../assets/images/select-image-extraction-properties-dark.gif#only-dark)
    ![Select Properties of Image Fields](../assets/images/select-image-extraction-properties-light.gif#only-light)
    <figcaption>Select Specific Properties of Interest For Relevant Fields</figcaption>
    </figure>

??? "3. Edit Property Configuration"
    To make additional properties visible (or to hide existing properties), the configuration can be edited.

    This will add/remove properties. The example below shows how to add the `size` property
    to the image in the `locator` field. The properties shown in purple
    are the automatically extracted properties.

    <figure markdown>
    ![Edit Which Properties Are Visible](../assets/images/edit-image-extraction-properties-dark.gif#only-dark)
    ![Edit Which Properties Are Visible](../assets/images/edit-image-extraction-properties-light.gif#only-light)
    <figcaption>Example of adding `size` to the list of extracted properties</figcaption>
    </figure>

!!! example

    <figure markdown>
    ![Hydrated Question](../assets/images/hydrated-image-example-light.png#only-light)
    ![Hydrated Question](../assets/images/hydrated-image-example-dark.png#only-dark)
    <figcaption>Example of a Hydrated Image - Note that the
        purple property indicates that they are auto-extracted </figcaption>
    </figure>

## Available Image Properties

The following properties are available for automatic image property extraction:

| Feature Name                  | Brief Description                              |
|-------------------------------|------------------------------------------------|
| [Aspect Ratio](#aspect-ratio) | Ratio of image width to height |
| [Brightness](#brightness) | Average pixel intensity |
| [Contrast](#contrast) | Standard deviation of pixel intensity |
| [Height](#height) | Height of image in pixels |
| [Pixel Entropy](#pixel-entropy) | Entropy of color distribution |
| [Sharpness](#sharpness) | Edge density from Canny edge detection |
| [Size](#size) | Product of image height and width |
| [Symmetry](#symmetry) | Level of vertical symmetry in image |
| [Width](#width) | Width of image in pixels |

## Feature Descriptions

### Aspect Ratio

**Aspect ratio** measures the ratio of the image width to its height. It can be useful in scenarios
where the image shape or dimensions impact the analysis or model performance.

$$
\text{Aspect Ratio} = \frac{\text{Width}}{\text{Height}}
$$

!!! example

    <figure markdown>
    ![Navigating to Configuration](../assets/images/extraction-aspect-dark.gif#only-dark)
    ![Navigating to Configuration](../assets/images/extraction-aspect-light.gif#only-light)
    <figcaption>The above example illustrates variation in aspect ratio</figcaption>
    </figure>

### Brightness

**Brightness** measures the average pixel intensity of an image, which can indicate how light or dark the image appears.
It can be useful in scenarios where the brightness impact the analysis or model performance.

$$
\text{Brightness} = \frac{\sum \text{Pixel Intensities}}{\text{Number of Pixels} \times \text{255}}
$$

!!! example

    <figure markdown>
    ![Navigating to Configuration](../assets/images/extraction-brightness-dark.gif#only-dark)
    ![Navigating to Configuration](../assets/images/extraction-brightness-light.gif#only-light)
    <figcaption>The above example illustrates variation in brightness</figcaption>
    </figure>

### Contrast

**Contrast** measures the standard deviation of pixel intensities in an image, indicating the degree of variation
between light and dark areas. The standard-deviation is normalized and bounded by a constant 200.
This value ranges from 0 - 1
with larger values denoting higher contrast in an image. This can be useful in scenarios where the contrast
impacts the analysis or model performance.

$$
\text{Contrast} = \min\left(1, \frac{\sigma_{\text{Pixel Intensities}}}{\text{200}}\right)
$$

!!! example

    <figure markdown>
    ![Navigating to Configuration](../assets/images/extraction-contrast-dark.gif#only-dark)
    ![Navigating to Configuration](../assets/images/extraction-contrast-light.gif#only-light)
    <figcaption>The above example illustrates variation in contrast</figcaption>
    </figure>

### Height

**Height** measures the height of the image in pixels. This is a straightforward dimension indicating the number
of pixel rows in the image. This enables analyzing any behaviors that vary with changing height.

$$
\text{Height} = \text{Number of Pixel Rows}
$$

### Pixel Entropy

**Pixel entropy** measures the entropy of the color distribution in an image, providing a measure of the image's
complexity or randomness. Higher entropy indicates more complexity. This can allow understanding of model behavior at
varying levels of complexity.

$$
\text{Pixel Entropy} = H\left(\frac{c_i}{\sum_{i=1}^{N} c_i}\right)
$$

$$
\text{where } c_i \text{ is the count of unique pixel } i, \text{ and } H \text{ denotes the Shannon entropy.}
$$

!!! example

    <figure markdown>
    ![Navigating to Configuration](../assets/images/extraction-entropy-dark.gif#only-dark)
    ![Navigating to Configuration](../assets/images/extraction-entropy-light.gif#only-light)
    <figcaption>The above example illustrates variation in entropy</figcaption>
    </figure>

### Sharpness

**Sharpness** measures the edge density in an image using the Canny edge detection algorithm. This can indicate how clear
or blurred an image is. The value is the proportion of edge pixels to total pixels. This can be useful in identifying
any discrepancies in model performance as it pertains to the blurriness or sharpness of an image.

$$
\text{Sharpness} = \frac{\text{Number of Edge Pixels}}{\text{Total Number of Pixels}}
$$

!!! example

    <figure markdown>
    ![Navigating to Configuration](../assets/images/extraction-sharpness-dark.gif#only-dark)
    ![Navigating to Configuration](../assets/images/extraction-sharpness-light.gif#only-light)
    <figcaption>Illustration of the difference between images with low sharpness and high sharpness</figcaption>
    </figure>

### Size

**Size** measures the product of the image height and width, giving the total number of pixels in the image.

$$
\text{Size} = \text{Height} \times \text{Width}
$$

### Symmetry

**Symmetry** measures the level of horizontal symmetry in an image by comparing the left and right halves.
The value ranges from 0 to 1, with 1 indicating perfect symmetry. This can highlight any behavioral differences
in data that is more symmetrical in nature.

$$
\text{Symmetry} = 1 - \frac{\text{MSE(Left Half, Mirrored Right Half)}}{\text{15000}}
$$

!!! example

    <figure markdown>
    ![Navigating to Configuration](../assets/images/extraction-symmetry-dark.gif#only-dark)
    ![Navigating to Configuration](../assets/images/extraction-symmetry-light.gif#only-light)
    <figcaption>The above example illustrates variation in symmetry</figcaption>
    </figure>

### Width

**Width** measures the width of the image in pixels. This is a straightforward dimension indicating the number of
pixel columns in the image. This enables inspecting any behaviors that vary with changing width.

$$
\text{Width} = \text{Number of Pixel Columns}
$$
