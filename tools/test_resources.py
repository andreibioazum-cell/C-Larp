#!/usr/bin/env python3
"""Проверяет ресурсы игры и схему сетевого хранилища."""

from __future__ import annotations

import json
import re
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def body_keys(name: str) -> set[str]:
    text = (ROOT / "src" / "engine" / "network" / name).read_text(encoding="utf-8")
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
    assert {"gbx", "gby", "gbdx", "gbdy", "grab"} <= room

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
    assert {"astra", "astra_level", "astra_levels"} <= profile

    missing = body_keys("player_api.inc") - message
    assert not missing, f"В правилах чата нет полей: {sorted(missing)}"
    missing = body_keys("room_control.inc") - banner
    assert not missing, f"В правилах баннера нет полей: {sorted(missing)}"


def check_assets() -> None:
    names: set[str] = set()
    game = ROOT / "src" / "game"
    for directory in (game / "core", game / "ui", game / "combat", game / "fx", game / "state"):
        for source in directory.rglob("*.inc"):
            names |= set(re.findall(r'"([A-Za-z0-9_./-]+\.png)"', source.read_text(encoding="utf-8")))
    assert names, "В исходниках не найдены ссылки на текстуры"
    missing = [
        name for name in sorted(names)
        if not (ROOT / "assets" / "textures" / name.split("/")[-1]).is_file()
    ]
    assert not missing, f"Не найдены текстуры: {missing}"

    required = (
        ROOT / "assets" / "audio" / "lobbymusic.wav",
        ROOT / "assets" / "audio" / "winter_jingle.wav",
        ROOT / "assets" / "audio" / "astra_azum_showdown.wav",
        ROOT / "assets" / "fonts" / "ComicRelief-Regular.ttf",
        ROOT / "assets" / "shaders" / "sprite.vert",
        ROOT / "assets" / "shaders" / "solid.frag",
        ROOT / "assets" / "shaders" / "image.frag",
        ROOT / "assets" / "shaders" / "tint.frag",
    )
    for asset in required:
        assert asset.is_file(), f"Не найден ресурс: {asset.relative_to(ROOT)}"

    durations = {
        "lobbymusic.wav": 35,
        "winter_jingle.wav": 35,
        "astra_azum_showdown.wav": 45,
    }
    for name, expected in durations.items():
        path = ROOT / "assets" / "audio" / name
        with wave.open(str(path), "rb") as audio:
            assert audio.getnchannels() == 2 and audio.getsampwidth() == 2, f"Неверный формат музыки: {name}"
            duration = audio.getnframes() / audio.getframerate()
            assert abs(duration - expected) < 0.001, f"Неверная длительность {name}: {duration}"


def check_port_layout() -> None:
    assert not list(ROOT.rglob("*.ds")), "В C-порте остались исходники старого языка"
    assert not (ROOT / "game").exists(), "Старый каталог game не должен использоваться"
    assert not (ROOT / "native").exists(), "Старый каталог native не должен использоваться"
    required = (
        ROOT / "src" / "game",
        ROOT / "src" / "engine",
        ROOT / "src" / "platform" / "android",
        ROOT / "assets" / "textures",
        ROOT / "assets" / "audio",
        ROOT / "assets" / "fonts",
        ROOT / "assets" / "shaders",
        ROOT / "platform" / "android",
    )
    for directory in required:
        assert directory.is_dir(), f"Нет каталога проекта: {directory.relative_to(ROOT)}"

    old_prefix = "ds" + "_fn_"
    for source in (ROOT / "src" / "game").rglob("*.inc"):
        assert old_prefix not in source.read_text(encoding="utf-8"), source


def main() -> int:
    check_port_layout()
    check_assets()
    check_firebase_rules()
    print("Ресурсы и схема сети: норма")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
