<br/>
<p align="center">
    <img src="https://sttci.b-cdn.net/status.swiftlycs2.net/2105/logo.png" alt="Swiftly Private Message Logo" width="600" height="131">
  <p align="center">
    Swiftly - Weapon Restrictor Extension
    <br/>
    <a href="https://github.com/swiftly-solution/base-extension/issues">Report Bug</a>
    <a href="https://swiftlys2.net/discord">Discord Server</a>
  </p>
</p>

<div align="center">

![Downloads](https://img.shields.io/github/downloads/swiftly-solution/base-extension/total) ![Contributors](https://img.shields.io/github/contributors/swiftly-solution/base-extension?color=dark-green) ![Issues](https://img.shields.io/github/issues/swiftly-solution/base-extension) ![License](https://img.shields.io/github/license/swiftly-solution/base-extension)

</div>

---

### Installation 👀

1. Download the newest [release](https://github.com/swiftly-solution/weapon-restrictor/releases)
2. Everything is drag & drop, so I think you can do it!

### Configuring the plugin 🧐

- After installing the plugin, you can change the prefix and the weapon restrict limits for each weapon from `addons/swiftly/configs/plugins/weapon_restrictor.json`

### Restriction Modes ⚙️

- You can choose between two restriction modes, `per_team` or `per_player`.

### Creating A Pull Request 😃

1. Fork the Project
2. Create your Feature Branch
3. Commit your Changes
4. Push to the Branch
5. Open a Pull Request

### Have ideas/Found bugs? 💡

Join [Swiftly Discord Server](https://swiftlycs2.net/discord) and send a message in the topic from `📕╎plugins-sharing` of this plugin!

---

### Build Requirements
-   [hl2sdk](https://github.com/alliedmodders/hl2sdk/tree/cs2) (Downloads automatically with the git cloning using Recurse Submodules)
-   [metamod-source](https://github.com/alliedmodders/metamod-source) (Downloads automatically with the git cloning using Recurse Submodules)
-   [XMake](https://xmake.io/)
---
### For Developers
- [Documentation](https://swiftlys2.net/ext-docs)
---
### Building Commands

#### Clone Repository

```
git clone --recurse-submodules https://github.com/swiftly-solution/weapon-restrictor
```

#### Build

```
./setup.ps1 - Windows
./setup.sh - Linux
```

#### Build using Docker

```
docker run --rm -it -e "FOLDER=ext" -e "GAME=cs2" -v .:/ext ghcr.io/swiftly-solution/swiftly:cross-compiler
```