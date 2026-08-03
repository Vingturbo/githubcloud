# GitHubCloud - 基于GitHub Release的在线文件存储

## We follow the GPLv3 license, so please don't use the GitHub API for anything shady.
## 我们遵循GPLv3协议，请不要使用GitHubAPI为非作歹

**GitHubCloud** is a command-line tool for managing files via **GitHub Releases**. It supports uploading, downloading (with automatic 7‑zip volume splitting for files larger than 2 GiB), searching, and a multilingual help system. All file metadata is stored in an `index.json` file that lives in your GitHub repository, so your file list stays synchronized across devices.

**GitHubCloud** 是一个基于 **GitHub Releases** 的文件管理命令行工具。支持上传、下载（大于 2 GiB 的文件自动 7z 分卷）、搜索，以及多语言帮助系统。所有文件的元数据存储在仓库中的 `index.json` 里，多端同步，随时获取最新文件列表。

---

## 📦 Installation / 安装

### Prerequisites / 依赖

- **Python** 3.8 or later / 3.8 或更高版本
- **p7zip** (for volume splitting and extraction) / 用于分卷压缩和解压
- **requests** (Python HTTP library) / Python HTTP 库

### Install on Different Platforms / 不同平台安装步骤

#### 🐧 Arch Linux

```bash
sudo pacman -S python python-requests p7zip
```

#### 🐧 Debian / Ubuntu

```bash
sudo apt update
sudo apt install python3 python3-requests p7zip-full
```

#### 🐧 Fedora

```bash
sudo dnf install python3 python3-requests p7zip
```

#### 🐧 openSUSE

```bash
sudo zypper install python3 python3-requests p7zip
```

#### 🍎 macOS (using Homebrew)

```bash
brew install python3 p7zip
pip3 install requests
```

#### 🪟 Windows

1. Download and install [Python 3](https://www.python.org/downloads/) (ensure “Add Python to PATH” is checked).
2. Install 7‑Zip from [https://www.7-zip.org/](https://www.7-zip.org/) and make sure `7z.exe` is in your `PATH`.
3. Install requests:

```cmd
pip install requests
```

4. 下载并安装 [Python 3](https://www.python.org/downloads/)（安装时勾选“Add Python to PATH”）。
5. 从 [https://www.7-zip.org/](https://www.7-zip.org/) 安装 7‑Zip，并确保 `7z.exe` 位于 `PATH` 环境变量中。
6. 安装 requests 库：

```cmd
pip install requests
```

---

## ⚙️ Configuration / 配置

Open `githubcloud.py` and modify the global variables at the top of the file:  
打开 `githubcloud.py`，修改文件顶部全局变量：

```python
GITHUB_TOKEN  = "ghp_xxxxxxxxxxxxx"   # 你的 GitHub Token
GITHUB_OWNER  = "Vingturbo"           # 仓库所有者
GITHUB_REPO   = "githubcloud"         # 仓库名
RELEASE_TAG   = "files"               # 默认 Release 标签
VOLUME_SIZE   = 1610612736            # 分块大小 (1.5 GiB)
INDEX_PATH    = "index.json"          # 索引文件路径
LANG_PATH     = "lang.json"           # 语言文件路径
```

You can also place a custom `lang.json` in your repository to display help in your preferred language.  
你也可以在仓库中放入自定义的 `lang.json` 来显示母语帮助信息。

---

## 🚀 Usage / 使用说明

```bash
python githubcloud.py [options]
```

### Options / 选项

| Option / 选项 | Description / 描述 |
|---------------|-------------------|
| `-h`          | Show help message (supports i18n) / 显示帮助信息（支持多语言） |
| `-u FILE`     | Upload a file / 上传文件 |
| `-d FILE`     | Download a file / 下载文件 |
| `-o DIR`      | Output directory for download (default: `.`) / 下载目录（默认当前目录） |
| `-s KEYWORD`  | Search non‑volume files / 搜索非分块文件 |

### Examples / 示例

#### Upload a file / 上传文件

```bash
python githubcloud.py -u myfile.txt
```

- If the file already exists and the SHA256 matches, it is skipped.  
  如果文件已存在且 SHA256 相同，则跳过。
- If the SHA256 differs, the file is renamed with a random prefix.  
  如果 SHA256 不同，则重命名（添加随机前缀）。
- Files larger than 2 GiB are automatically split into 1.5 GiB 7‑zip volumes and uploaded.  
  大于 2 GiB 的文件会自动 7z 分卷（每卷 1.5 GiB）并上传。

#### Download a file / 下载文件

```bash
python githubcloud.py -d myfile.txt -o ./downloads
```

- Volume files are automatically merged and extracted using 7‑zip.  
  分卷文件会自动合并并解压。

#### Search files / 搜索文件

```bash
python githubcloud.py -s keyword
```

- Searches only non‑volume file names (the original file name, not volume parts).  
  仅搜索非分块文件名（搜索原始文件名，不包含分卷）。

#### Show help / 显示帮助

```bash
python githubcloud.py -h
```

---

## 📁 `index.json` Structure / 结构

The tool reads / writes `index.json` from the repository root.  
工具会读写仓库根目录下的 `index.json`。

```json
{
  "file_list": [
    {
      "name": "example.txt",
      "size": 1024,
      "volume": false,
      "upload_time": 1723456789000,
      "sha256": "abc123..."
    },
    {
      "name": "large.iso",
      "size": 3000000000,
      "volume": true,
      "upload_time": 1723456789000,
      "sha256": "def456...",
      "volumes": {
        "001": {
          "name": "large.iso.7z.001",
          "size": 1610612736
        },
        "002": {
          "name": "large.iso.7z.002",
          "size": 1389387264
        }
      }
    }
  ]
}
```

---

## 🌐 Internationalization / 多语言

Place a `lang.json` file in your repository to customize help messages. The default language is English.  
在仓库中放置 `lang.json` 可以自定义帮助文本。默认语言为英语。

Example `lang.json`:  
`lang.json` 示例：

```json
{
  "help_desc": "通过 GitHub Release 管理文件",
  "h_help": "显示帮助信息",
  "u_help": "上传文件",
  "d_help": "下载文件",
  "o_help": "指定下载目录",
  "s_help": "搜索文件",
  "unknown_opt": "未知选项，请使用 -h 查看帮助",
  "upload_ok": "上传成功",
  "upload_dup": "文件已存在且内容相同，跳过",
  "upload_rename": "文件已存在但内容不同，重命名为：",
  "download_ok": "下载完成",
  "download_not_found": "未在索引中找到文件",
  "search_result": "搜索结果",
  "search_none": "未找到匹配的文件"
}
```

---

## ⚠️ Notes / 注意事项

- SSL verification is globally disabled because some environments may lack CA certificates.  
  由于某些环境可能缺少 CA 证书，工具全局禁用了 SSL 验证。
- Make sure your GitHub Token has `repo` scope (for private repos) or `public_repo` (for public repos).  
  请确保 GitHub Token 拥有 `repo`（私有仓库）或 `public_repo`（公开仓库）权限。
- The tool uses the GitHub REST API; be aware of [rate limits](https://docs.github.com/en/rest/overview/resources-in-the-rest-api#rate-limiting).  
  工具使用 GitHub REST API，请注意[速率限制](https://docs.github.com/en/rest/overview/resources-in-the-rest-api#rate-limiting)。

---

Enjoy your permanent, CDN‑backed file archive!  
享受你的永久、可 CDN 加速的文件存档吧！
