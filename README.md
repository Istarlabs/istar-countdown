<!--
SPDX-FileCopyrightText: 2026 Pascal Klose
SPDX-License-Identifier: MIT
-->

# istar-countdown

Countdowns, a timer and a working-time calculator, in Kirigami so they look like the rest of a Plasma
session. The countdowns are yours to write; the big gradient timer text also works as a stream overlay.

- **Countdowns** - a list of named events with a date, a time, how often they repeat (once, weekly,
  every second week, monthly, yearly, birthday) and the message shown when they are over.
  Starting one counts down to its *next* occurrence.
- **Timer** - the ticking text, filled with a two-colour gradient. The finished message and both
  colours are remembered.
- **Working time** - start, pause and target length give the end of the day; the end can be changed
  and the time actually worked is shown. One button counts down to that end.

Translated into English (the source language), German and Japanese.

## Build

Needs Qt 6.7+, KF6 (CoreAddons, Config, I18n, Kirigami), `kf6-kirigami-addons` at run time (date picker, form cards, about page),
ECM and a C++26 compiler. On Fedora: `qt6-qtbase-devel`, `qt6-qtdeclarative-devel`,
`kf6-ki18n-devel`, `kf6-kirigami-devel`, `extra-cmake-modules`, `kf6-kirigami-addons`.
`clang-tools-extra` for `Scripts/cpp.py format` and clang-tidy.

The scripts in `Scripts/` come from [devscripts](https://github.com/Istarlabs/devscripts), pinned as a submodule: clone
with `git clone --recurse-submodules`, or run `git submodule update --init` in an existing clone.

```bash
python3 Scripts/cpp.py verify      # clean build + tests + format + licence lines
python3 Scripts/cpp.py run
python3 Scripts/cpp.py install     # to ~/.local
```

## RPM

```bash
python3 Scripts/rpm.py build       # ~/.cache/devscripts/rpm/istar-countdown-<version>-<timestamp>.x86_64.rpm
python3 Scripts/rpm.py publish     # the same, sent to an RPM repository over ssh (see below)
```

devscripts' `rpm.py` builds it from `[rpm]` in `Scripts/project.toml`, in a build directory of its own
(`build/rpm`): Release, prefix `/usr`, no tests. The libraries it needs are found by `rpmbuild`; what
QML loads at run time (Kirigami, kirigami-addons, the desktop style, Qt Declarative) and the icon theme
are listed by name. `publish` needs a repository: `[rpm] repository = "host:path"` or `DEVSCRIPTS_RPM_REPOSITORY`.
A build installed into `~/.local` before stays in front of the package: `python3 Scripts/cpp.py uninstall`.

The translations are compiled by `install`, into `share/locale` of the prefix. A binary started from
`build/bin` has no `.mo` files next to it and shows English; the installed one follows the session
language.
