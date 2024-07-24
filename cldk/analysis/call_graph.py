from abc import ABC, abstractmethod


class CallGraph(ABC):
    def __init__(self) -> None:
        super().__init__()

    @abstractmethod
    def get_callees(self, **kwargs):
        """
        Given a source code, get all the callees
        """
        pass

    @abstractmethod
    def get_callers(self, **kwargs):
        """
        Given a source code, get all the callers
        """
        pass

    @abstractmethod
    def get_call_graph(self, **kwargs):
        """
        Given an application, get the call graph
        """
        pass

    @abstractmethod
    def get_call_graph_json(self, **kwargs):
        """
        Given an application, get call graph in JSON format
        """
        pass

    @abstractmethod
    def get_class_call_graph(self, **kwargs):
        """
        Given an application and a class, get call graph
        """
        pass

    @abstractmethod
    def get_entry_point_classes(self, **kwargs):
        """
        Given an application, get all the entry point classes
        """
        pass

    @abstractmethod
    def get_entry_point_methods(self, **kwargs):
        """
        Given an application, get all the entry point methods
        """
        pass

    @abstractmethod
    def get_service_entry_point_classes(self, **kwargs):
        """
        Given an application, get all the service entry point classes
        """
        pass

    @abstractmethod
    def get_service_entry_point_methods(self, **kwargs):
        """
        Given an application, get all the service entry point methods
        """
        pass
