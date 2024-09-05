import json
import re
from typing import Dict, List, Optional
from pydantic import BaseModel, field_validator, model_validator
from contextvars import ContextVar
from .constants_namespace import ConstantsNamespace

constants = ConstantsNamespace()
context_concrete_class = ContextVar("context_concrete_class")  # context var to store class concreteness


class JField(BaseModel):
    """
    Represents a field in a Java class or interface.

    Parameters
    ----------
    comment : str
        The comment associated with the field.
    name : str
        The name of the field.
    type : str
        The type of the field.
    start_line : int
        The starting line number of the field in the source file.
    end_line : int
        The ending line number of the field in the source file.
    variables : List[str]
        The variables declared in the field.
    modifiers : List[str]
        The modifiers applied to the field (e.g., public, static).
    annotations : List[str]
        The annotations applied to the field.
    """

    comment: str
    type: str
    start_line: int
    end_line: int
    variables: List[str]
    modifiers: List[str]
    annotations: List[str]


class JCallableParameter(BaseModel):
    """
    Represents a parameter of a Java callable.

    Parameters
    ----------
    name : str
        The name of the parameter.
    type : str
        The type of the parameter.
    """

    name: str
    type: str
    annotations: List[str]
    modifiers: List[str]


class JEnumConstant(BaseModel):
    name: str
    arguments: List[str]


class JCallSite(BaseModel):
    """
    Represents a call site.

    Parameters
    ----------
    method_name : str
        The name of the method called at the call site.
    receiver_expr : str
        Expression for the receiver of the method call.
    receiver_type : str
        Name of type declaring the called method.
    argument_types : List[str]
        Types of actual parameters for the call.
    return_type : str
        Return type of the method call (resolved type of the method call expression; empty string if expression is
        unresolved).
    is_static_call: bool
        Flag indicating whether the call is a static call
    is_constructor_call: bool
        Flag indicating whether the call is a constructor call
    start_line : int
        The starting line number of the call site.
    start_column : int
        The starting column of the call site.
    end_line : int
        The ending line number of the call site.
    end_column : int
        The ending column of the call site.
    """

    method_name: str
    receiver_expr: str = ""
    receiver_type: str
    argument_types: List[str]
    return_type: str = ""
    callee_signature: str = ""
    is_static_call: bool | None = None
    is_private: bool | None = None
    is_public: bool | None = None
    is_protected: bool | None = None
    is_unspecified: bool | None = None
    is_constructor_call: bool
    start_line: int
    start_column: int
    end_line: int
    end_column: int


class JVariableDeclaration(BaseModel):
    """
    Represents a variable declaration.

    Parameters
    ----------
    name : str
        The name of the variable.
    type : str
        The type of the variable.
    initializer : str
        The initialization expression (if persent) for the variable declaration.
    start_line : int
        The starting line number of the declaration.
    start_column : int
        The starting column of the declaration.
    end_line : int
        The ending line number of the declaration.
    end_column : int
        The ending column of the declaration.
    """

    name: str
    type: str
    initializer: str
    start_line: int
    start_column: int
    end_line: int
    end_column: int


class JCallable(BaseModel):
    """
    Represents a callable entity such as a method or constructor in Java.

    Parameters
    ----------
    signature : str
        The signature of the callable.
    is_implicit : bool
        A flag indicating whether the callable is implicit (e.g., a default constructor).
    is_constructor : bool
        A flag indicating whether the callable is a constructor.
    comment : str
        The comment associated with the callable.
    annotations : List[str]
        The annotations applied to the callable.
    modifiers : List[str]
        The modifiers applied to the callable (e.g., public, static).
    thrown_exceptions : List[str]
        Exceptions declared via "throws".
    declaration : str
        The declaration of the callable.
    parameters : List[ParameterInCallable]
        The parameters of the callable.
    return_type : Optional[str]
        The return type of the callable. None if the callable does not return a value (e.g., a constructor).
    code : str
        The code block of the callable.
    start_line : int
        The starting line number of the callable in the source file.
    end_line : int
        The ending line number of the callable in the source file.
    referenced_types : List[str]
        The types referenced within the callable.
    accessed_fields : List[str]
        Fields accessed in the callable.
    call_sites : List[JCallSite]
        Call sites in the callable.
    variable_declarations : List[JVariableDeclaration]
        Local variable declarations in the callable.
    cyclomatic_complexity : int
        Cyclomatic complexity of the callable.
    """

    signature: str
    is_implicit: bool
    is_constructor: bool
    is_entry_point: bool = False
    comment: str
    annotations: List[str]
    modifiers: List[str]
    thrown_exceptions: List[str] = []
    declaration: str
    parameters: List[JCallableParameter]
    return_type: Optional[str] = None  # Pythonic way to denote a nullable field
    code: str
    start_line: int
    end_line: int
    referenced_types: List[str]
    accessed_fields: List[str]
    call_sites: List[JCallSite]
    variable_declarations: List[JVariableDeclaration]
    cyclomatic_complexity: int | None

    def __hash__(self):
        return hash(self.declaration)

    @model_validator(mode="after")
    def detect_entrypoint_method(self):
        # check first if the class in which this method exists is concrete or not, by looking at the context var
        if context_concrete_class.get():
            # convert annotations to the form GET, POST even if they are @GET or @GET('/ID') etc.
            annotations_cleaned = [match for annotation in self.annotations for match in
                                   re.findall(r'@(.*?)(?:\(|$)', annotation)]

            param_type_list = [val.type for val in self.parameters]
            # check the param types against known servlet param types
            if any(substring in string for substring in param_type_list for string in
                   constants.ENTRY_POINT_METHOD_SERVLET_PARAM_TYPES):
                # check if this method is over-riding (only methods that override doGet / doPost etc. will be flagged as first level entry points)
                if 'Override' in annotations_cleaned:
                    self.is_entry_point = True
                    return self

            # now check the cleaned annotations against known javax ws annotations
            if any(substring in string for substring in annotations_cleaned for string in
                   constants.ENTRY_POINT_METHOD_JAVAX_WS_ANNOTATIONS):
                self.is_entry_point = True
                return self

            # check the cleaned annotations against known spring rest method annotations
            if any(substring in string for substring in annotations_cleaned for string in
                   constants.ENTRY_POINT_METHOD_SPRING_ANNOTATIONS):
                self.is_entry_point = True
                return self
        return self


class JType(BaseModel):
    """
    Represents a Java class or interface.

    Parameters
    ----------
    name : str
        The name of the class or interface.
    is_interface : bool
        A flag indicating whether the object is an interface.
    is_inner_class : bool
        A flag indicating whether the object is an inner class.
    is_local_class : bool
        A flag indicating whether the object is a local class.
    is_nested_type: bool
        A flag indicating whether the object is a nested type.
    comment : str
        The comment of the class or interface.
    extends_list : List[str]
        The list of classes or interfaces that the object extends.
    implements_list : List[str]
        The list of interfaces that the object implements.
    modifiers : List[str]
        The list of modifiers of the object.
    annotations : List[str]
        The list of annotations of the object.
    parent_class:
        The name of the parent class (if it exists)
    nested_class_declerations: List[str]
        All the class declerations nested under this class.
    constructor_declarations : List[JCallable]
        The list of constructors of the object.
    method_declarations : List[JCallable]
        The list of methods of the object.
    field_declarations : List[JField]
        The list of fields of the object.
    """

    is_interface: bool = False
    is_inner_class: bool = False
    is_local_class: bool = False
    is_nested_type: bool = False
    is_class_or_interface_declaration: bool = False
    is_enum_declaration: bool = False
    is_annotation_declaration: bool = False
    is_record_declaration: bool = False
    is_concrete_class: bool = False
    is_entry_point: bool = False
    comment: str
    extends_list: List[str] = []
    implements_list: List[str] = []
    modifiers: List[str] = []
    annotations: List[str] = []
    parent_type: str
    nested_type_declerations: List[str] = []
    callable_declarations: Dict[str, JCallable] = {}
    field_declarations: List[JField] = []
    enum_constants: List[JEnumConstant] = []

    # first get the data in raw form and check if the class is concrete or not, before any model validation is done
    #   for this we assume if a class is not an interface or abstract it is concrete
    #       for abstract classes we will check the modifiers
    @model_validator(mode="before")
    def check_concrete_class(cls, values):
        values["is_concrete_class"] = False
        if values.get("is_class_or_interface_declaration") and not values.get("is_interface"):
            if "abstract" not in values.get("modifiers"):
                values["is_concrete_class"] = True
        # since the methods in this class need access to the concrete class flag,
        # we will store this in a context var - this is a hack
        token = context_concrete_class.set(values["is_concrete_class"])
        return values

    # after model validation is done we populate the is_entry_point flag by checking
    # if the class extends or implements known servlet classes
    @model_validator(mode="after")
    def check_concrete_entry_point(self):
        if self.is_concrete_class:
            if any(substring in string for substring in (self.extends_list + self.implements_list)
                   for string in constants.ENTRY_POINT_SERVLET_CLASSES):
                self.is_entry_point = True
                return self
        # Handle spring classes
        # clean annotations - take out @ and any paranehesis along with info in them.
        annotations_cleaned = [match for annotation in self.annotations for match in
                               re.findall(r'@(.*?)(?:\(|$)', annotation)]
        if any(substring in string for substring in annotations_cleaned
               for string in constants.ENTRY_POINT_CLASS_SPRING_ANNOTATIONS):
            self.is_entry_point = True
            return self
        # context_concrete.reset()
        return self


class JCompilationUnit(BaseModel):
    comment: str
    imports: List[str]
    type_declarations: Dict[str, JType]


class JMethodDetail(BaseModel):
    method_declaration: str
    # class is a reserved keyword in python. we'll use klass.
    klass: str
    method: JCallable

    def __repr__(self):
        return f"JMethodDetail({self.method_declaration})"

    def __hash__(self):
        return hash(tuple(self))


class JGraphEdgesST(BaseModel):
    source: JMethodDetail
    target: JMethodDetail
    type: str
    weight: str
    source_kind: str | None = None
    destination_kind: str | None = None


class JGraphEdges(BaseModel):
    source: JMethodDetail
    target: JMethodDetail
    type: str
    weight: str
    source_kind: str | None = None
    destination_kind: str | None = None

    @field_validator("source", "target", mode="before")
    @classmethod
    def validate_source(cls, value) -> JMethodDetail:
        callable_dict = json.loads(value)
        j_callable = JCallable(**json.loads(callable_dict["callable"]))  # parse the value which is a quoted string
        class_name = callable_dict["class_interface_declarations"]
        method_decl = j_callable.declaration
        mc = JMethodDetail(method_declaration=method_decl, klass=class_name, method=j_callable)
        return mc

    def __hash__(self):
        return hash(tuple(self))


class JApplication(BaseModel):
    """
    Represents a Java application.

    Parameters
    ----------
    symbol_table : List[JCompilationUnit]
        The symbol table representation
    system_dependency : List[JGraphEdges]
        The edges of the system dependency graph. Default None.
    """
    symbol_table: Dict[str, JCompilationUnit]
    system_dependency_graph: List[JGraphEdges] = None
