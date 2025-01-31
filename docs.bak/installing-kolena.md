---
icon: kolena/developer-16
---

# :kolena-developer-20: Installing `kolena`

[`kolena SDK`](https://github.com/kolenaIO/kolena) is a powerful tool that enables process automation and data curation.

`kolena` is released under the open-source [Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0)
license. The package is [hosted on PyPI](https://pypi.org/project/kolena/) and can be installed using your
preferred Python package manager.

## Installation

SDK builds can be installed directly from
[PyPI](https://pypi.org/project/kolena/) using any Python package manager such as [pip](https://pypi.org/project/pip/)
or [uv](https://docs.astral.sh/uv/):

=== "`pip`"

    ```shell
    pip install kolena
    ```

=== "`uv`"

    ```shell
    uv add kolena
    ```

#### Extra Dependency Groups

Certain metrics computation functionality depends on additional packages like
[scikit-learn](https://scikit-learn.org/stable/). These extra dependencies can be installed via the `metrics` group:

=== "`pip`"

    ```shell
    pip install 'kolena[metrics]'
    ```

=== "`uv`"

    ```shell
    uv add 'kolena[metrics]'
    ```

## Initialization

When using `kolena`, sessions are automatically authenticated using any token present in the `KOLENA_TOKEN` environment
variable or in your `.netrc` file.

To get an API token, visit the [:kolena-developer-16: Developer](https://app.kolena.com/redirect/developer) page to
generate a token, then set the `KOLENA_TOKEN` variable in your environment:

```bash
export KOLENA_TOKEN="********"
```

By default, sessions are automatically initialized when a function requiring initialization is called. To configure
your session, e.g. to disable extra logging via `verbose=False`, you can manually initialize by calling
[`kolena.initialize(...)`](./reference/initialize.md) directly:

```python
import kolena

kolena.initialize(verbose=False)
```

By default, sessions have static scope and persist until the interpreter is exited. See the documentation on
[`initialize`](./reference/initialize.md) for details.

!!! tip "Tip: `logging`"

    Integrate `kolena` into your existing logging system by filtering for events from the `"kolena"` logger. All log
    messages are emitted as both Python standard library [`logging`][logging] events as well as stdout/stderr messages.

## Supported Python Versions

`kolena` is compatible with all active Python versions.

| :fontawesome-brands-python: Python Version                        | :kolena-logo: Compatible `kolena` Versions |
|-------------------------------------------------------------------|--------------------------------------------|
| 3.12                                                              | ≥1.3                                       |
| 3.11                                                              | ≥0.69                                      |
| 3.10                                                              | _All Versions_                             |
| 3.9                                                               | _All Versions_                             |
| 3.8                                                               | _All Versions_                             |
| 3.7 (EOL: [June 2023](https://devguide.python.org/versions/))     | ≤0.99                                      |
| 3.6 (EOL: [December 2021](https://devguide.python.org/versions/)) | ≤0.46                                      |
