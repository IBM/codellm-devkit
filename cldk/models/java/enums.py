from enum import Enum


class CRUDOperationType(Enum):
    """An enumeration of CRUD operation types.

    Attributes:
        CREATE (str): The create operation type.
        READ (str): The read operation type.
        UPDATE (str): The update operation type.
        DELETE (str): The delete operation type.
    """
    CREATE = "CREATE"
    READ   = "READ"
    UPDATE = "UPDATE"
    DELETE = "DELETE"

class CRUDQueryType(Enum):
    """An enumeration of CRUD query types.

    Attributes:
        READ (str): The read query type.
        WRITE (str): The write query type.
        NAMED (str): The named query type.
    """
    READ   = "READ"
    WRITE  = "WRITE"
    NAMED = "NAMED"