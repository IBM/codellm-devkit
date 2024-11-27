################################################################################
# Copyright IBM Corporation 2024
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

"""
Constants Namespace module
"""


class ConstantsNamespace:
    @property
    def ENTRY_POINT_SERVLET_CLASSES(self):
        return ("javax.servlet.GenericServlet", "javax.servlet.Filter", "javax.servlet.http.HttpServlet")

    @property
    def ENTRY_POINT_METHOD_SERVLET_PARAM_TYPES(self):
        return ("javax.servlet.ServletRequest", "javax.servlet.ServletResponse", "javax.servlet.http.HttpServletRequest", "javax.servlet.http.HttpServletResponse")

    @property
    def ENTRY_POINT_METHOD_JAVAX_WS_ANNOTATIONS(self):
        return ("javax.ws.rs.POST", "javax.ws.rs.PUT", "javax.ws.rs.GET", "javax.ws.rs.HEAD", "javax.ws.rs.DELETE")

    @property
    def ENTRY_POINT_CLASS_SPRING_ANNOTATIONS(self):
        return ("Controller", "RestController")

    @property
    def ENTRY_POINT_METHOD_SPRING_ANNOTATIONS(self):
        return ("GetMapping", "PathMapping", "PostMapping", "PutMapping", "RequestMapping", "DeleteMapping")
