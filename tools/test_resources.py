#!/usr/bin/env python3
"""Проверяет ресурсы игры и схему сетевого хранилища."""

from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def body_keys(name: str) -> set[str]:
    text = (ROOT / "native" / "net" / name).read_text(encoding="utf-8")
    return set(re.findall(r'\\"([a-z0-9_]+)\\":', text))


def declared(node: dict) -> set[str]:
    return {key for key in node if not key.startswith((".", "$"))}


def check_firebase_rules() -> None:
    rules = json.loads((ROOT / "firebase.rules.json").read_text(encoding="utf-8"))["rules"]
    slot = declared(rules["rooms"]["$room"]["players"]["$slot"])
    user = declared(rules["users"]["$nick"])
    banner = declared(rules["banner"])
    message = declared(rules["rooms"]["$room"]["chat"]["$msg"])

    room = body_keys("room_sync.inc") | body_keys("room_chat.inc") | body_keys("room_threads.inc")
    missing = room - slot
    assert not missing, f"В правилах игрового слота нет полей: {sorted(missing)}"

    auth_only = {"email", "password", "grant_type", "refresh_token"}
    profile = (
        body_keys("state_storage.inc")
        | body_keys("cloud_patch.inc")
        | body_keys("settings_storage.inc")
        | body_keys("promo.inc")
        | (body_keys("auth_session.inc") - auth_only)
    )
    missing = profile - user
    assert not missing, f"В правилах профиля нет полей: {sorted(missing)}"

    missing = body_keys("player_api.inc") - message
    assert not missing, f"В правилах чата нет полей: {sorted(missing)}"
    missing = body_keys("room_control.inc") - banner
    assert not missing, f"В правилах баннера нет полей: {sorted(missing)}"


def check_assets() -> None:
    names: set[str] = set()
    for directory in (ROOT / "game" / "modules", ROOT / "game" / "state"):
        for source in directory.rglob("*.inc"):
            names |= set(re.findall(r'"([A-Za-z0-9_./-]+\.png)"', source.read_text(encoding="utf-8")))
    assert names, "В исходниках не найдены ссылки на текстуры"
    missing = [
        name for name in sorted(names)
        if not (ROOT / "game" / "assets" / name.split("/")[-1]).is_file()
    ]
    assert not missing, f"Не найдены текстуры: {missing}"


def check_port_layout() -> None:
    assert not list(ROOT.rglob("*.ds")), "В C-порте остались исходники старого языка"
    old_prefix = "ds" + "_fn_"
    for source in (ROOT / "game").rglob("*.inc"):
        assert old_prefix not in source.read_text(encoding="utf-8"), source


def main() -> int:
    check_port_layout()
    check_assets()
    check_firebase_rules()
    print("Ресурсы и схема сети: норма")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
