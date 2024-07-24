class ConstantsNamespace:
    @property
    def ENTRY_POINT_SERVLET_CLASSES(self):
        return ("javax.servlet.GenericServlet","javax.servlet.Filter","javax.servlet.http.HttpServlet")
    @property
    def ENTRY_POINT_METHOD_SERVLET_PARAM_TYPES(self):
        return ("javax.servlet.ServletRequest","javax.servlet.ServletResponse","javax.servlet.http.HttpServletRequest","javax.servlet.http.HttpServletResponse")
    @property
    def ENTRY_POINT_METHOD_JAVAX_WS_ANNOTATIONS(self):
        return ("javax.ws.rs.POST", "javax.ws.rs.PUT", "javax.ws.rs.GET", "javax.ws.rs.HEAD", "javax.ws.rs.DELETE")
    @property
    def ENTRY_POINT_CLASS_SPRING_ANNOTATIONS(self):
        return ("Controller","RestController")
    @property
    def ENTRY_POINT_METHOD_SPRING_ANNOTATIONS(self):
        return ("GetMapping","PathMapping","PostMapping","PutMapping","RequestMapping","DeleteMapping")