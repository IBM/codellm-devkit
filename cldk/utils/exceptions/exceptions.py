class CldkInitializationException(Exception):
    """Custom exception for errors during CLDK initialization."""

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)


class CodeanalyzerExecutionException(Exception):
    """Exception raised for errors that occur during the execution of Codeanalyzer."""

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)


class CodeQLDatabaseBuildException(Exception):
    """Exception raised for errors that occur during the building of a CodeQL database."""

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)


class CodeQLQueryExecutionException(Exception):
    """Exception raised for errors that occur during the execution of a CodeQL query."""

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)


class CodeanalyzerUsageException(Exception):
    """
    Exception is raised when the usage of codeanalyzer is incorrect.
    """

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)
