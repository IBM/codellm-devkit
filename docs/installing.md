---
icon: cldk/developer-20
---

# :cldk-developer-20: Installing `cldk`

[`CLDK`](https://github.com/IBM/codellm-devkit) is a Python SDK [hosted on PyPI](https://pypi.org/project/cldk/) and can be installed using your preferred Python package manager.

## Installation

The Python SDK can be installed directly from
[PyPI](https://pypi.org/project/cldk/) using any Python package manager such as [pip](https://pypi.org/project/pip/), [poetry](https://python-poetry.org/), or [uv](https://docs.astral.sh/uv/):

=== "`pip`"

    ```shell
    pip install cldk
    ```

=== "`poetry`"

    ```shell
    poetry add cldk
    ```

=== "`uv`"

    ```shell
    uv add cldk
    ```

## Extra Dependencies 


`CLDK` supports program analysis for multiple languages and requires additional dependencies to support specific languages. The following table lists the additional dependencies required for each language:


### Java Analysis
    
For Java analysis, CLDK relies on a companion project called [`codeanalyzer`](https://github.com/ibm/codenet-minerva-code-analyzer). `codeanalyzer` is a java project and you will therefore need to install the Java Development Kit (JDK) with java version 11 or later. 

You can use a package manager like [SDKMAN](https://sdkman.io/) to install the JDK. First, install SDKMAN by running the following command:

- Install SDKMan!
   Open your terminal and enter the following command:

    ```bash
    curl -s "https://get.sdkman.io" | bash
    ```

    Follow the on-screen instructions to complete the installation.

- Open a new terminal or source the SDKMan! scripts:

    ```bash
    source "$HOME/.sdkman/bin/sdkman-init.sh"
    ```

Next, install java 11 or later using SDKMAN:

- You can list all available java versions with:

    ```bash
    sdk list java | grep sem
    ```

    You should see the following:
    ```bash
        Semeru      |     | 21.0.5       | sem     |            | 21.0.5-sem
                    |     | 17.0.13      | sem     |            | 17.0.13-sem
                    |     | 11.0.25      | sem     |            | 11.0.25-sem
                    |     | 8.0.432      | sem     |            | 8.0.432-sem
    ```

- Install Java 11 or above (we'll go with 11.0.2-sem):

    ```bash
    sdk install java 11.0.25-sem
    ```

- Set Java 11 as the current (or default) Java version:

    ```bash
    sdk [use|default] java 11.0.25-sem
    ```

- Verify the installation:

    ```bash
    java -version
    ```

    This should output the version of the installed Java.
    
    ```bash
    openjdk 11.0.25 2024-10-15
    IBM Semeru Runtime Open Edition 11.0.25.0 (build 11.0.25+9)
    Eclipse OpenJ9 VM 11.0.25.0 (build openj9-0.48.0, JRE 11 Linux amd64-64-Bit Compressed References 20241107_1233 (JIT enabled, AOT enabled)
    OpenJ9   - 1d5831436e
    OMR      - d10a4d553
    JCL      - edded3f65c based on jdk-11.0.25+9)
    ```

Finally, to enable building Java projects automatically, you will need to install the `maven` build tool. You can install `maven` using a package manager like `SDKMAN`:

- Install Maven:

    ```bash
    sdk install maven
    ```

- Make sure `mvn` command is available in the `PATH`. Add the following to your `~/.zshrc`, `~/.bashrc` or `~/.bash_profile`:

    ```bash
    export PATH="$HOME/.sdkman/candidates/maven/current/bin:$PATH"
    ```

### Python Analysis

For Python analysis as well as to use the CLDK Python SDK, you will need to install the Python programming language with version 3.11 or later. We recommend using a package manager like [pyenv](https://github.com/pyenv/pyenv) to install and manage Python dependencies.

### C/C++ Analysis

CLDK uses LLVM and Clang Python bindings to analyze C/C++ code. The project requires specific versions:

- libclang >= 18.1.1
- clang >= 17.0.6

You can install LLVM and Clang using various package managers depending on your operating system.

=== "macOS"

    - Install LLVM 18 using Homebrew
    ```shell
    brew install llvm@18
    ```
    
    - Add LLVM to your PATH (add this to your ~/.zshrc or ~/.bash_profile)
    ```shell
    export PATH="/usr/local/opt/llvm@18/bin:$PATH"
    export LDFLAGS="-L/usr/local/opt/llvm@18/lib"
    export CPPFLAGS="-I/usr/local/opt/llvm@18/include"
    ```
    
    - Verify installation
    ```shell
    clang --version
    ```
    This should output the version of the installed LLVM and Clang.
    ```shell
    Apple clang version 18.1.1
    Target: x86_64-apple-darwin21.6.0
    Thread model: posix
    InstalledDir: /usr/local/opt/llvm@18/bin
    ```

=== "Ubuntu/Debian"

    - Add LLVM repository and install required packages
    ```shell
    wget https://apt.llvm.org/llvm.sh
    chmod +x llvm.sh
    sudo ./llvm.sh 18
    sudo apt-get install llvm-18 llvm-18-dev clang-18 libclang-18-dev
    ```
    
    - Create symlinks (optional but recommended)
    ```shell
    sudo ln -s /usr/bin/clang-18 /usr/bin/clang
    sudo ln -s /usr/bin/llvm-config-18 /usr/bin/llvm-config
    ```
    
    - Verify installation
    ```shell
    clang --version
    ```
    This should output the version of the installed LLVM and Clang.
    ```shell
    Ubuntu clang version 18.1.1
    Target: x86_64-pc-linux-gnu
    Thread model: posix
    InstalledDir: /usr/bin
    ```

=== "Red Hat-based Systems (Fedora/CentOS/RHEL)"

    - Install LLVM 18 and development packages
    ```shell
    # On Fedora
    sudo dnf install llvm18 llvm18-devel clang18 clang18-devel

    # On CentOS/RHEL (if needed)
    sudo yum install epel-release
    sudo yum install llvm18 llvm18-devel clang18 clang18-devel
    ```
    
    - Create symlinks (optional but recommended)
    ```shell
    sudo ln -s /usr/bin/clang-18 /usr/bin/clang
    sudo ln -s /usr/bin/llvm-config-18 /usr/bin/llvm-config
    ```
    
    - Verify installation
    ```shell
    clang --version
    ```
    This should output the version of the installed LLVM and Clang.
    ```shell
    clang version 18.1.8 (Fedora 18.1.8-5.fc41)
    Target: x86_64-redhat-linux-gnu
    Thread model: posix
    InstalledDir: /usr/bin
    Configuration file: /etc/clang18/x86_64-redhat-linux-gnu-clang.cfg
    ```


### Additional Notes

Some operating systems may require additional development tools:

=== "macOS"

    - Make sure you have the Xcode Command Line Tools installed. You can install them using the following command:

        ```shell
        xcode-select --install
        ```

    - Additionally, you may need to install the following packages using Homebrew:
        
        ```shell
        brew install openssl readline sqlite3 xz zlib tcl-tk libffi
        ```

=== "Ubuntu/Debian"

    - Install the required development tools using the following command:

        ```shell
        sudo apt-get install build-essential python3-dev libssl-dev zlib1g-dev \
        libbz2-dev libreadline-dev libsqlite3-dev curl git \
        libncursesw5-dev xz-utils tk-dev libxml2-dev libxmlsec1-dev libffi-dev liblzma-dev
        ```

=== "Red Hat-based Systems (Fedora/CentOS/RHEL)"

    - Install the required development tools using the following command:
        
        ```shell
        sudo dnf group install c-development development-tools gcc make \
        patch zlib-devel bzip2 bzip2-devel readline-devel sqlite sqlite-devel\ 
        openssl-devel tk-devel libffi-devel xz-devel
        ```



## Supported Python Versions

`CLDK` is compatible with Python versions 3.11 and later. The following table lists the supported Python versions and the corresponding `CLDK` versions:

| :fontawesome-brands-python: Python Version                        | :cldk-logo-white: Compatible `cldk` Versions |
|-------------------------------------------------------------------|--------------------------------------------|
| 3.11                                                              | ≥0.4.0                                      |
