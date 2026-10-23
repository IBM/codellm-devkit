"""Go model tests — the pydantic mirror parses a real cango v2 payload, and the null-safe base
coerces a JSON ``null`` collection to its empty default (Go's nil-slice idiom)."""

import json
from pathlib import Path

from cldk.models.go import GoAnalysis, GoModule, GoType

FIXTURE = Path(__file__).parent / "fixture_analysis.json"


def test_fixture_validates():
    a = GoAnalysis(**json.loads(FIXTURE.read_text(encoding="utf-8")))
    assert a.schema_version == "2.0.0"
    assert a.language == "go"
    assert a.max_level == 2
    assert a.application.id.startswith("can://go/")
    assert a.application.symbol_table


def test_gotype_is_a_single_model_with_a_kind_field():
    t = GoType(id="can://go/x/f.go/T", kind="struct")
    assert t.kind == "struct"
    assert t.base_types == [] and t.callables == {} and t.fields == {}


def test_null_safe_base_coerces_null_collections():
    """A JSON ``null`` list/map (Go marshals a nil slice this way) becomes the empty default rather
    than raising, honoring the ``= []`` field style via the broadened validator."""
    t = GoType(id="x", kind="interface", base_types=None, callables=None, fields=None)
    assert t.base_types == [] and t.callables == {} and t.fields == {}
    m = GoModule(id="m", package="p", source="", imports=None, types=None, functions=None)
    assert m.imports == [] and m.types == {} and m.functions == {}


def test_go_native_fields_present():
    """The Go-specific first-class fields exist with their defaults."""
    from cldk.models.go import GoBodyNode, GoCallable, GoParam

    assert GoParam(name="x").is_variadic is False
    assert GoBodyNode(kind="call").is_goroutine is False
    c = GoCallable(id="c", signature="s")
    assert c.error_channel == [] and c.source_file is None
