---
icon: kolena/properties-16
---

# :kolena-properties-20: Automatically Extract Bounding Box Properties

To help with understanding of data and discovering untested scenarios, Kolena automatically extracts a number of
properties from [bounding boxes](../reference/annotation.md#kolena.annotation.BoundingBox) used to mark
ground truth or model results.

The extraction happens automatically when bounding boxes are uploaded onto the platform. This document
outlines what they are and how to configure them.

## Configuring Bounding Box Property Extraction

Bounding box extraction happens automatically on Kolena when data is uploaded or modified.
To configure the properties extracted, navigate to the extractions tab in your dataset and
select which extractions you are interested in.

!!! Note
    It is best practice to group bounding boxes of interest separately. For example, when uploading model results, you should
    group your True Positive, False Negative and False Positive bounding boxes in separate lists.

<figure markdown>
![Configuring Bounding Box Extractions](../assets/images/boundingbox-extraction-configuration-dark.gif#only-dark)
![Configuring Bounding Box Extractions](../assets/images/boundingbox-extraction-configuration-light.gif#only-light)
<figcaption>Access Bounding Box extraction configuration</figcaption>
</figure>

## Available Bounding Box Properties

The following properties are available for automatic bounding box property extraction. There are two types of properties:

1. Bounding Box Property: the property related to a single bounding box
2. Bounding Box Group Property: the property related to the group of bounding boxes uploaded in a list of bounding boxes.

| Feature Name                                                                      | Brief Description                                                                                    | Property Type              |
|-----------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------|----------------------------|
| [Aspect Ratio](#aspect-ratio)                                                     | The aspect ratio of the bounding box (`width / height`)                                                | Bounding Box Property |
| [Relative Area](#relative-area)                                                   | The relative area of the bounding box with respect to the image (`area of bbox / area of image`)         | Bounding Box Property |
| [Relative Distance To Image Center](#relative-distance-to-image-center)           | The Euclidean distance between the bounding box's center and the center of the image                 | Bounding Box Property |
| [Object Count](#object-count)                                                     | Count the number of bounding boxes in the group                                                      | Bounding Box Group Property |
| [Object Type Count](#object-type-count)                                           | Count the types of objects in the group according to label field                                     | Bounding Box Group Property |
| [Combined Area Ratio](#combined-area-ratio)                                       | Sum of the bounding box size ratio in the group (`sum(bounding box area / image area)`)                                                           | Bounding Box Group Property |
| [Bbox Intersection](#bbox-intersection)                                            | Sum of the IOU of all bounding box pairs in the group                                                | Bounding Box Group Property |
| [Entropy Of Distribution](#entropy-of-distribution)                               | Entropy of the distribution of bounding boxes within a 3 by 3 grid                                   | Bounding Box Group Property |
| [Mean Aspect Ratio](#mean-aspect-ratio)                                           | The mean of the aspect ratios of a group of bounding boxes                                           | Bounding Box Group Property |
| [Mean Distance Between Bboxes](#mean-distance-between-bboxes)                     | The mean of the Euclidean distance between each pair of bounding box in the group                    | Bounding Box Group Property |
| [Mean Relative Area](#mean-relative-area)                                         | The mean of the relative area of bounding boxes in the group                                         | Bounding Box Group Property |
| [Mean Relative Distance To Image Center](#mean-relative-distance-to-image-center) | The mean of the relative distance between each of the bounding box and the image center in the group | Bounding Box Group Property |
| [Std Aspect Ratio](#std-aspect-ratio)                                             | The standard deviation of the aspect ratios of a group of bounding boxes                             | Bounding Box Group Property |
| [Std Distance Between Bboxes](#std-distance-between-bboxes)                      | The standard deviation  of the Euclidean distance between each pair of bounding box in the group     | Bounding Box Group Property |
| [Std Relative Area](#std-relative-area)                                          | The standard deviation of the relative area of bounding boxes in the group                                         | Bounding Box Group Property |
| [Std Relative Distance To Image Center](#std-relative-distance-to-image-center)  | The standard deviation of the relative distance between each of the bounding box and the image center in the group | Bounding Box Group Property |

## Feature Descriptions

### Aspect Ratio

**Aspect Ratio** measures the ratio of the bounding box width to its height. It can be useful in scenarios
where the bounding box shape or dimensions impact the analysis or model performance.

$$
\text{Aspect Ratio} = \frac{\text{Bounding Box Width}}{\text{Bounding Box Height}}
$$

### Relative Area

**Relative Area** measures the ratio of the bounding box area to the area of the image its referenced to.
It can be useful to identify relatively large or small bounding boxes.

$$
\text{Relative Area} = \frac{\text{Bounding Box Area}}{\text{Image Area}}
$$

### Relative Distance To Image Center

**Relative Distance To Image Center** measures the distance between the center of the bounding box and center
of the image it is referencing. It can be used to monitor model performance based on how far the object is from
the center of the image.

### Object Count

**Object Count** is a simple count of bounding boxes in a group of bounding boxes. You can use this value
to monitor how your model performance changes when there are a few or many objects to detect in an image.

### Object Type Count

**Object Type Count** counts the number of labels in a list of bounding boxes. You can use this count to
see how your models perform when classifying a few or many different objects in a scene.

### Combined Area Ratio

**Combined Area Ratio** is the sum of area bounding box area ratios (sum(bounding box area/image area)).

### Bbox Intersection

**Bbox Intersection** is the sum of Intersection over union ratios of pairs of bounding boxes in a group bounding boxes.
This property is a good indication of potential duplicate detections or density of overlapping bounding boxes in an image.

### Mean Aspect Ratio

**Mean Aspect Ratio** is the average of aspect ratios of bounding boxes in a list of bounding boxes. You can use this property
if your model performance may depend on shape of the objects in the image.

### Entropy Of Distribution

**Entropy Of Distribution** is a measure of how spread apart bounding boxes in a given group are. It is
a good measure to see if model performs differently based on how the bounding boxes are spread in a scene.

### Mean Distance Between Bboxes

**Mean Distance Between Bboxes** is the mean of the Euclidean distance between each pair of bounding box in the group.
You can use this property to develope a sense for density of bounding boxes and how that relates to model performance.

### Mean Relative Area

**Mean Relative Area** is the average of relative area described above.

### Mean Relative Distance To Image Center

**Mean Relative Distance To Image Center** is the average of Euclidean distance between the center of each bounding
box and the center of the image.

### Std Aspect Ratio

**Std Aspect Ratio** is the standard deviation of aspect ratio in a list of bounding boxes.

### Std Distance Between Bboxes

**Std Distance Between Bboxes** is the standard deviation of distance of each pair of bounding boxes in a list
of bounding boxes.

### Std Relative Area

**Std Relative Area** is the standard deviation of relative area of a bounding box over the image in a list of bounding boxes.

### Std Relative Distance To Image Center

**Std Relative Distance To Image Center** is the standard deviation of Euclidean distance between the center of
a bounding box to the center of the image in a list of bounding boxes.
