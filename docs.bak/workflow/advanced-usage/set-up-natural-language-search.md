---
icon: kolena/comparison-16
search:
  boost: -0.5
---

# :kolena-comparison-20: Natural Language Search Setup

Kolena supports natural language and similar image search
across [`Image`][kolena.workflow.Image] data previously registered to the platform.
Users may set up this functionality by extracting and
uploading the corresponding search embeddings using a Kolena provided package.

## Example

The [`kolena`](https://github.com/kolenaIO/kolena) repository contains a runnable
[example](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/search_embeddings) for embeddings extraction and
upload. This builds off the data uploaded in the
[age_estimation](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/age_estimation)
example workflow, and is best run after this data has been uploaded to your Kolena environment.

## How to Set Up Natural Language Search

Uploading embeddings to Kolena can be done in three simple steps:

- [**Step 1**](#step-1-install-kolena_embeddings-package): installing dependency package
- [**Step 2**](#step-2-load-images-for-extraction): loading images for input to extraction library
- [**Step 3**](#step-3-extract-and-upload-embeddings): extracting and uploading search embeddings

Let's take a look at each step with example code snippets.

### Step 1: Install `kolena_embeddings` Package

The package can be installed via `pip` or `uv` and requires use of your kolena token which can be created
on the [:kolena-developer-16: Developer](https://app.kolena.com/redirect/developer) page.

=== "`pip`"

    ```shell
    pip install --extra-index-url="https://MY_KOLENA_TOKEN@gateway.kolena.cloud/repositories" kolena-embeddings
    ```

=== "`uv`"

    ```shell
    uv add --extra-index-url="https://MY_KOLENA_TOKEN@gateway.kolena.cloud/repositories" kolena-embeddings
    ```

This package provides the `kembed.util.extract_and_upload_embeddings` method:

```python
from typing import Iterable
from typing import Tuple

from PIL import Image

def extract_and_upload_embeddings(locators_and_images: Iterable[Tuple[str, Image.Image]], batch_size: int = 50) -> None:
    """
    Extract and upload a list of search embeddings corresponding to sample locators.
    Expects to have an exported `KOLENA_TOKEN` environment variable, as per [Kolena client documentation](https://docs.kolena.com/installing-kolena/#initialization).

    :param locators_and_images: An iterator through PIL Image files and their corresponding locators (as provided to
        the Kolena platform).
    :param batch_size: Batch size for number of images to extract embeddings for simultaneously. Defaults to 50 to
        avoid having too many file handlers open at once.
    """
```

### Step 2: Load Images for Extraction

In order to extract embeddings on image data, we must load our image files into a
[`PIL.Image.Image`](https://pillow.readthedocs.io/en/stable/reference/Image.html#PIL.Image.Image) object.
In this section, we will load these images from an S3 bucket.
For other cloud storage services, please refer to your cloud storage's API docs.

```python
from typing import Iterator
from typing import List
from typing import Tuple

import boto3
from urllib.parse import urlparse
from PIL import Image

s3 = boto3.client("s3")

def load_image_from_locator(locator: str) -> Image.Image:
    parsed_url = urlparse(locator)
    bucket_name = parsed_url.netloc
    key = parsed_url.path.lstrip("/")
    file_stream = boto3.resource("s3").Bucket(bucket_name).Object(key).get()["Body"]
    return Image.open(file_stream)

def iter_image_locators(locators: List[str]) -> Iterator[Tuple[str, Image.Image]]:
    for locator in locators:
        image = load_image_from_locator(locator)
        yield locator, image
```

!!! tip end
    When processing large scales of images, we recommend using an `Iterator` to limit the number
    of images loaded into memory at once.

### Step 3: Extract and Upload Embeddings

We first [retrieve and set](../../installing-kolena.md#initialization) our `KOLENA_TOKEN` environment variable.
This is used by the uploader for authentication against your Kolena instance.

```shell
export KOLENA_TOKEN="********"
```

We then pass our locators into the `extract_and_upload_embeddings` function to iteratively upload embeddings for all
[`Image`][kolena.workflow.Image] objects in the Kolena platform with matching locators.

```python
from kembed.util import extract_and_upload_embeddings

locators = [
    "s3://kolena-public-datasets/labeled-faces-in-the-wild/imgs/AJ_Cook/AJ_Cook_0001.jpg",
    "s3://kolena-public-datasets/labeled-faces-in-the-wild/imgs/AJ_Lamas/AJ_Lamas_0001.jpg",
    "s3://kolena-public-datasets/labeled-faces-in-the-wild/imgs/Aaron_Eckhart/Aaron_Eckhart_0001.jpg",
]
extract_and_upload_embeddings(iter_image_locators(locators))
```

Once the upload completes, we can now visit [<nobr>:kolena-studio-16: Studio</nobr>](https://app.kolena.com/redirect/studio)
to search by natural language over the corresponding [`Image`][kolena.workflow.Image] data.

## Conclusion

In this tutorial, we learned how to extract and upload vector embeddings over your [`Image`][kolena.workflow.Image] data.

## FAQ

??? faq "Can I share embeddings with Kolena even if I do not share the underlying images?"
    Yes!

    Embeddings extraction is a unidirectional mapping, and used only for natural language search and similarity comparisons.
    Uploading these embeddings to Kolena does not allow for any reconstruction of these images, nor does it involve
    sharing these images with Kolena.

??? faq "Do I need to upload embeddings for every test suite on the Kolena platform?"
    Embeddings are uploaded by locator, but resolved against existing image samples in the platform at upload time.
    This means that these embeddings are matched against every image with the provided locator across multiple test
    suites, and subsequent test suites containing the same [`Image`][kolena.workflow.Image] will remain associated
    with these search embeddings.

    Please note that if you subsequently register an image sample with different fields (but the same locator),
    the previously uploaded embeddings may not automatically associate with the image sample.
    We are working on improving this process so that once embeddings are uploaded once, future image samples linked
    to the same locator will automatically use these embeddings. Please stay tuned!
