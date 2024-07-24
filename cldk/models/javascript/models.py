from typing import List, Optional
from pydantic import BaseModel


class JsParameter(BaseModel):
    """
    Represents a function/method parameter in Javascript.

    Parameters
    ----------
    name : str
        The name of the parameter.
    default_value : Optional[str]
        The default value of the parameter, used when a value is not provided.
    is_rest : bool
        A flag indicating whether the parameter is a rest parameter.
    """

    name: str
    default_value: Optional[str] = None
    is_rest: bool = False
    # type: Optional[str] - might be able to extract from JsDoc


class JsCallable(BaseModel):
    """
    Represents a callable entity such as a method or function in Javascript.

    Parameters
    ----------
    name : str
        The name of the callable.
    signature : str
        The signature of the callable.
    parameters : List[JsParameter]
        The parameters of the callable.
    code : str
        The code block of the callable.
    start_line : int
        The starting line number of the callable in the source file.
    end_line : int
        The ending line number of the callable in the source file.
    is_constructor : bool
        A flag indicating whether the callable is a constructor.
    """

    name: str
    code: str
    signature: str
    paremeters: List[JsParameter]
    start_line: int
    end_line: int
    is_constructor: bool = False


class JsClass(BaseModel):
    """
    Represents a class in Javascript.

    Parameters
    ----------
    name : str
        The name of the class.
    methods : List[JsCallable]
        The methods of the class.
    start_line : int
        The starting line number of the class in the source file.
    end_line : int
        The ending line number of the class in the source file.
    parent : Optional[str]
        The name of the parent class.
    """

    name: str
    methods: List[JsCallable]
    parent: Optional[str] = None
    start_line: int
    end_line: int


class JsProgram(BaseModel):
    """
    Represents a source file in Javascript.

    Parameters
    ----------
    classes : List[JsClass]
        The list of classes present in the source file.
    callables : List[JsCallable]
        The list of callable entities present in the source file.
    """

    classes: List[JsClass]
    callables: List[JsCallable]
    # TODO: imports
