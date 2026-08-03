#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
GitHubCloud - GitHub Release 文件管理命令行工具
所有配置在代码开头的全局变量区修改
"""

import argparse
import base64
import hashlib
import json
import locale
import os
import platform
import random
import subprocess
import sys
import time
from pathlib import Path

import urllib3
import requests

# ═══════════════════════════════════════════════════════════════
# 全局禁用 SSL 验证
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
requests.packages.urllib3.disable_warnings()

# 创建全局会话（后续所有 HTTP 请求共用）
session = requests.Session()
session.verify = False
# ═══════════════════════════════════════════════════════════════

# ═══════════════════ 全局配置（修改此处） ═══════════════════
GITHUB_TOKEN = "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"          # GitHub 个人访问令牌
GITHUB_OWNER = "Vingturbo"                 # 仓库所有者
GITHUB_REPO  = "githubcloud"               # 仓库名
RELEASE_TAG  = "files"                     # 默认 Release 标签
VOLUME_SIZE  = 1610612736                  # 分块大小 (1.5 GiB)
INDEX_PATH   = "index.json"                # 仓库中索引文件路径
LANG_PATH    = "lang.json"                 # 仓库中语言文件路径
SEVEN_ZIP    = "7z"                        # 7z 命令，Windows 可能需要指定完整路径如 "C:\\Program Files\\7-Zip\\7z.exe"
# ═══════════════════════════════════════════════════════════════

# ═══════════════════ 全局变量（运行时缓存） ═══════════════════
_LANG_DATA = None       # 缓存语言数据，避免重复加载
_SYSTEM_LOCALE = None   # 缓存系统区域设置

# ═══════════════════ 调试与日志 ═══════════════════
def debug(msg):
    """输出带时间戳的调试信息"""
    ts = time.strftime("%Y-%m-%d %H:%M:%S")
    print(f"[{ts}] {msg}")

# ═══════════════════ 跨平台工具函数 ═══════════════════
def get_program_dir():
    """获取程序文件所在目录（不是运行目录）"""
    return os.path.dirname(os.path.abspath(__file__))

def get_disk_usage(path):
    """获取路径所在磁盘的可用空间（字节）"""
    try:
        if platform.system() == "Windows":
            import ctypes
            free_bytes = ctypes.c_ulonglong(0)
            ctypes.windll.kernel32.GetDiskFreeSpaceExW(
                ctypes.c_wchar_p(path), None, None, ctypes.pointer(free_bytes))
            return free_bytes.value
        else:
            st = os.statvfs(path)
            return st.f_bavail * st.f_frsize
    except:
        return -1

def human_size(size_bytes):
    """将字节数转换为人类可读的大小"""
    for unit in ("B", "KiB", "MiB", "GiB", "TiB"):
        if abs(size_bytes) < 1024.0:
            return f"{size_bytes:.1f} {unit}"
        size_bytes /= 1024.0
    return f"{size_bytes:.1f} PiB"

def get_system_locale():
    """获取系统区域设置，默认返回 zh_CN.UTF-8"""
    global _SYSTEM_LOCALE
    if _SYSTEM_LOCALE:
        return _SYSTEM_LOCALE

    try:
        loc = locale.getlocale()[0]
        if loc:
            _SYSTEM_LOCALE = loc
            return _SYSTEM_LOCALE
    except:
        pass

    # 尝试从环境变量获取
    for var in ("LANG", "LC_ALL", "LC_CTYPE"):
        val = os.environ.get(var)
        if val:
            _SYSTEM_LOCALE = val
            return _SYSTEM_LOCALE

    _SYSTEM_LOCALE = "zh_CN.UTF-8"
    return _SYSTEM_LOCALE

def random_md5():
    """生成 0~2147483647 随机数的 MD5 哈希"""
    return hashlib.md5(str(random.randint(0, 2147483647)).encode()).hexdigest()

def file_sha256(path):
    """计算文件的 SHA256"""
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        while True:
            chunk = f.read(8192)
            if not chunk:
                break
            h.update(chunk)
    return h.hexdigest()

# ═══════════════════ 进度条 ═══════════════════
def print_progress(current, total, prefix=""):
    """打印简单进度条"""
    bar_len = 40
    if total > 0:
        percent = current / total
        filled = int(bar_len * percent)
        bar = '█' * filled + '░' * (bar_len - filled)
        print(f"\r{prefix} |{bar}| {percent*100:.1f}%", end='', flush=True)
    if current >= total:
        print()  # 换行

# ═══════════════════ 多语言支持 ═══════════════════
def load_language():
    """
    加载语言文件：优先本地程序目录，其次仓库根目录。
    结果缓存到全局变量 _LANG_DATA，避免重复加载和重复输出调试信息。
    """
    global _LANG_DATA
    if _LANG_DATA is not None:
        return _LANG_DATA

    lang_data = {}

    # 1. 尝试加载本地 lang.json（程序所在目录）
    local_lang = os.path.join(get_program_dir(), "lang.json")
    if os.path.exists(local_lang):
        try:
            with open(local_lang, 'r', encoding='utf-8') as f:
                lang_data = json.load(f)
            debug(f"使用本地语言文件: {local_lang}")
        except Exception as e:
            debug(f"读取本地 lang.json 失败: {e}")

    # 2. 如果本地没有，尝试从仓库根目录拉取
    if not lang_data:
        debug("本地未找到 lang.json，尝试从仓库加载...")
        url = f"https://api.github.com/repos/{GITHUB_OWNER}/{GITHUB_REPO}/contents/{LANG_PATH}"
        headers = {"Authorization": f"token {GITHUB_TOKEN}"}
        r = session.get(url, headers=headers)
        if r.status_code == 200:
            try:
                content = base64.b64decode(r.json()["content"]).decode("utf-8")
                lang_data = json.loads(content)
                debug("已从仓库加载语言文件")
            except Exception as e:
                debug(f"解析仓库 lang.json 失败: {e}")

    _LANG_DATA = lang_data
    return _LANG_DATA

def get_text(key):
    """
    根据 key 和系统区域设置获取对应的多语言文本。
    首次调用时加载语言文件并缓存。
    """
    lang = load_language()
    loc = get_system_locale()

    # 1. 精确匹配
    if loc in lang and key in lang[loc]:
        return lang[loc][key]

    # 2. 前缀匹配（如 zh_CN 匹配 zh）
    lang_prefix = loc.split("_")[0] if "_" in loc else loc
    for l, trans in lang.items():
        if l.startswith(lang_prefix) and key in trans:
            return trans[key]

    # 3. 回退到 zh_CN.UTF-8
    if "zh_CN.UTF-8" in lang and key in lang["zh_CN.UTF-8"]:
        return lang["zh_CN.UTF-8"][key]

    # 4. 最后回退到 key 本身
    return key

# ═══════════════════ GitHub API ═══════════════════
def get_release_assets():
    """获取 Release 的所有资产列表"""
    url = f"https://api.github.com/repos/{GITHUB_OWNER}/{GITHUB_REPO}/releases/tags/{RELEASE_TAG}"
    headers = {"Authorization": f"token {GITHUB_TOKEN}"}
    r = session.get(url, headers=headers)
    r.raise_for_status()
    return r.json().get("assets", [])

def get_upload_url():
    """获取 Release 的上传 URL"""
    url = f"https://api.github.com/repos/{GITHUB_OWNER}/{GITHUB_REPO}/releases/tags/{RELEASE_TAG}"
    headers = {"Authorization": f"token {GITHUB_TOKEN}"}
    r = session.get(url, headers=headers)
    r.raise_for_status()
    return r.json()["upload_url"]

def fetch_index():
    """从仓库拉取 index.json，不存在则返回初始结构"""
    url = f"https://api.github.com/repos/{GITHUB_OWNER}/{GITHUB_REPO}/contents/{INDEX_PATH}"
    headers = {"Authorization": f"token {GITHUB_TOKEN}"}
    r = session.get(url, headers=headers)
    if r.status_code == 200:
        content = base64.b64decode(r.json()["content"]).decode("utf-8")
        return json.loads(content)
    debug(get_text("index_not_found"))
    return {"file_list": []}

def push_index(index):
    """推送 index.json 到仓库（自动处理创建/更新）"""
    url = f"https://api.github.com/repos/{GITHUB_OWNER}/{GITHUB_REPO}/contents/{INDEX_PATH}"
    headers = {"Authorization": f"token {GITHUB_TOKEN}"}

    # 获取当前文件的 sha（如果存在）
    r = session.get(url, headers=headers)
    sha = None
    if r.status_code == 200:
        sha = r.json()["sha"]

    content = json.dumps(index, indent=2, ensure_ascii=False)
    payload = {
        "message": "Update index.json",
        "content": base64.b64encode(content.encode()).decode(),
    }
    if sha:
        payload["sha"] = sha

    r = session.put(url, headers=headers, json=payload)
    if r.status_code in (200, 201):
        debug(get_text("index_updated"))
    else:
        debug(get_text("index_update_failed").format(code=r.status_code, msg=r.text))

def upload_asset(upload_url, file_path, file_name):
    """上传单个文件到 Release，支持进度显示"""
    real_url = upload_url.replace("{?name,label}", f"?name={file_name}")
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Content-Type": "application/octet-stream"
    }

    file_size = os.path.getsize(file_path)
    uploaded = 0

    class ProgressUpload:
        def __init__(self, file_path):
            self.file = open(file_path, 'rb')
            self.size = os.path.getsize(file_path)
            self.read_so_far = 0

        def read(self, size=-1):
            data = self.file.read(size)
            self.read_so_far += len(data)
            print_progress(self.read_so_far, self.size, f"  {file_name}")
            return data

        def __len__(self):
            return self.size

        def close(self):
            self.file.close()

    progress = ProgressUpload(file_path)
    try:
        r = requests.post(
            real_url,
            headers=headers,
            data=progress,
            verify=False
        )
        return r
    finally:
        progress.close()

# ═══════════════════ 命令功能 ═══════════════════
def print_help():
    """打印帮助信息"""
    lang = load_language()
    loc = get_system_locale()
    debug(f"系统区域设置: {loc}")

    title = get_text("help_title")
    print(title)
    print(get_text("help_usage").format(prog=os.path.basename(sys.argv[0])))
    print(f"  -h            {get_text('help_h')}")
    print(f"  -u FILE       {get_text('help_u')}")
    print(f"  -d FILE       {get_text('help_d')}")
    print(f"  -o DIR        {get_text('help_o')}")
    print(f"  -s [KEYWORD]  {get_text('help_s')}")
    print(f"\n{get_text('help_examples')}:")
    print(f"  {sys.argv[0]} -u myfile.zip")
    print(f"  {sys.argv[0]} -d myfile.zip -o ./downloads")
    print(f"  {sys.argv[0]} -s keyword")
    print(f"  {sys.argv[0]} -s")

def cmd_upload(file_path):
    """上传文件"""
    debug(get_text("upload_start").format(path=file_path))

    if not os.path.exists(file_path):
        print(get_text("err_file_not_found").format(path=file_path))
        sys.exit(1)

    file_name = os.path.basename(file_path)
    file_size = os.path.getsize(file_path)
    file_hash = file_sha256(file_path)
    debug(f"  文件: {file_name}, 大小: {human_size(file_size)}, SHA256: {file_hash}")

    # 获取 Release 现有资产
    assets = get_release_assets()
    asset_names = {a["name"] for a in assets}
    debug(f"  Release 中已有 {len(assets)} 个文件")

    # 判断是否需要分块（大于 2GB）
    need_volume = file_size > 2 * 1024 * 1024 * 1024

    if need_volume:
        # 询问确认
        est_volumes = (file_size + VOLUME_SIZE - 1) // VOLUME_SIZE
        print(get_text("volume_confirm").format(
            name=file_name,
            count=est_volumes,
            size=human_size(VOLUME_SIZE)
        ))
        confirm = input(get_text("volume_confirm_prompt") + " [y/N] ").strip().lower()
        if confirm not in ('y', 'yes'):
            print(get_text("volume_cancel"))
            return

        debug(get_text("volume_start"))
        base_vol_name = f"{file_name}.7z"
        cmd = [SEVEN_ZIP, "a", f"-v{VOLUME_SIZE}b", base_vol_name, file_path]
        debug(f"  执行: {' '.join(cmd)}")
        subprocess.run(cmd, check=True)

        vol_files = sorted([f for f in os.listdir('.') if f.startswith(base_vol_name)])
        debug(f"  生成 {len(vol_files)} 个分卷")

        upload_url = get_upload_url()
        vol_info = {}
        for idx, vol_file in enumerate(vol_files, 1):
            debug(f"  上传分卷 {idx}/{len(vol_files)}: {vol_file}")

            # 检查重名
            final_name = vol_file
            if vol_file in asset_names:
                existing_sha = None
                index = fetch_index()
                for f in index.get("file_list", []):
                    if f.get("volume"):
                        for v in f["volumes"].values():
                            if v["name"] == vol_file:
                                existing_sha = v.get("sha256")
                                break
                if existing_sha and existing_sha == file_sha256(vol_file):
                    debug(f"    分卷已存在且哈希相同，跳过")
                    vol_info[f"{idx:03d}"] = {
                        "name": vol_file,
                        "size": os.path.getsize(vol_file),
                        "sha256": existing_sha
                    }
                    continue
                else:
                    final_name = f"({random_md5()}{file_sha256(vol_file)}){vol_file}"
                    debug(f"    重命名为: {final_name}")
                    os.rename(vol_file, final_name)

            r = upload_asset(upload_url, final_name, final_name)
            if r.status_code == 201:
                vol_hash = file_sha256(final_name)
                vol_info[f"{idx:03d}"] = {
                    "name": final_name,
                    "size": os.path.getsize(final_name),
                    "sha256": vol_hash
                }
                debug(f"    分卷上传成功")
            else:
                debug(f"    上传失败: {r.status_code} {r.text}")
                sys.exit(1)

        index = fetch_index()
        index["file_list"].append({
            "name": file_name,
            "size": file_size,
            "volume": True,
            "upload_time": int(time.time() * 1000),
            "sha256": file_hash,
            "volumes": vol_info
        })
        push_index(index)
        debug(get_text("upload_done"))

    else:
        # 普通文件上传
        final_name = file_name
        if file_name in asset_names:
            debug(f"  Release 中存在同名文件: {file_name}")
            index = fetch_index()
            existing_sha = None
            for f in index.get("file_list", []):
                if f["name"] == file_name and not f.get("volume"):
                    existing_sha = f.get("sha256")
                    break
            if existing_sha and existing_sha == file_hash:
                print(get_text("upload_dup"))
                return
            else:
                final_name = f"({random_md5()}{file_hash}){file_name}"
                print(get_text("upload_rename").format(new=final_name))
                debug(f"  重命名为: {final_name}")

        upload_url = get_upload_url()
        r = upload_asset(upload_url, file_path, final_name)
        if r.status_code == 201:
            debug(f"  文件 {final_name} 上传成功")
            index = fetch_index()
            index["file_list"].append({
                "name": final_name,
                "size": file_size,
                "volume": False,
                "upload_time": int(time.time() * 1000),
                "sha256": file_hash
            })
            push_index(index)
            print(get_text("upload_ok"))
        else:
            debug(f"  上传失败: {r.status_code} {r.text}")
            sys.exit(1)

def cmd_download(file_name, output_dir):
    """下载文件"""
    debug(get_text("download_start").format(name=file_name))
    index = fetch_index()

    target = None
    for f in index.get("file_list", []):
        if f["name"] == file_name and not f.get("volume"):
            target = f
            break
        if f.get("volume"):
            for v in f["volumes"].values():
                if v["name"] == file_name:
                    print(get_text("err_volume_only").format(main=f["name"]))
                    sys.exit(1)

    if not target:
        print(get_text("download_not_found"))
        sys.exit(1)

    os.makedirs(output_dir, exist_ok=True)

    if target.get("volume"):
        vols = target["volumes"]
        debug(f"  文件由 {len(vols)} 个分卷组成")
        for key in sorted(vols.keys()):
            vol = vols[key]
            vol_url = f"https://github.com/{GITHUB_OWNER}/{GITHUB_REPO}/releases/download/{RELEASE_TAG}/{vol['name']}"
            save_path = os.path.join(output_dir, vol["name"])
            debug(f"  下载分卷: {vol['name']} ({human_size(vol['size'])})")
            r = session.get(vol_url, headers={"Authorization": f"token {GITHUB_TOKEN}"}, stream=True)
            r.raise_for_status()
            total = int(r.headers.get('content-length', 0))
            downloaded = 0
            with open(save_path, 'wb') as fout:
                for chunk in r.iter_content(8192):
                    fout.write(chunk)
                    downloaded += len(chunk)
                    print_progress(downloaded, total, f"  {vol['name']}")

        # 合并解压
        first_vol = vols[min(vols.keys())]["name"]
        first_vol_path = os.path.join(output_dir, first_vol)
        debug(f"  合并解压: {first_vol_path}")
        cmd = [SEVEN_ZIP, "x", first_vol_path, f"-o{output_dir}", "-y"]
        subprocess.run(cmd, check=True)
        debug(get_text("download_done"))
    else:
        dl_url = f"https://github.com/{GITHUB_OWNER}/{GITHUB_REPO}/releases/download/{RELEASE_TAG}/{file_name}"
        save_path = os.path.join(output_dir, file_name)
        debug(f"  下载: {file_name} ({human_size(target['size'])})")
        r = session.get(dl_url, headers={"Authorization": f"token {GITHUB_TOKEN}"}, stream=True)
        r.raise_for_status()
        total = int(r.headers.get('content-length', 0))
        downloaded = 0
        with open(save_path, 'wb') as fout:
            for chunk in r.iter_content(8192):
                fout.write(chunk)
                downloaded += len(chunk)
                print_progress(downloaded, total, f"  {file_name}")
        debug(get_text("download_done"))

    print(get_text("download_ok"))

def cmd_search(keyword):
    """搜索文件"""
    index = fetch_index()
    if keyword is None:
        results = [f for f in index.get("file_list", []) if not f.get("volume")]
    else:
        results = [f for f in index.get("file_list", [])
                   if not f.get("volume") and keyword.lower() in f["name"].lower()]

    if results:
        print(get_text("search_result").format(count=len(results)))
        for f in results:
            size_str = human_size(f["size"])
            print(f"  {f['name']}  ({size_str})")
    else:
        print(get_text("search_none"))

# ═══════════════════ 主入口 ═══════════════════
def main():
    # 初始化语言（只加载一次）
    load_language()
    debug(f"系统区域设置: {get_system_locale()}")

    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument('-h', action='store_true')
    parser.add_argument('-u', type=str, metavar='FILE')
    parser.add_argument('-d', type=str, metavar='FILE')
    parser.add_argument('-o', type=str, metavar='DIR', default='.')
    parser.add_argument('-s', type=str, nargs='?', const=None, metavar='KEYWORD')

    # 检查未知参数
    known = {'-h', '-u', '-d', '-o', '-s'}
    for arg in sys.argv[1:]:
        if arg.startswith('-') and arg not in known:
            print(get_text("err_unknown_opt"))
            print(get_text("help_hint"))
            sys.exit(1)

    args = parser.parse_args()

    if args.h or (not args.u and not args.d and args.s is None and '-s' not in sys.argv):
        print_help()
        return

    try:
        if args.u:
            cmd_upload(args.u)
        elif args.d:
            cmd_download(args.d, args.o)
        elif '-s' in sys.argv:
            cmd_search(args.s)
    except KeyboardInterrupt:
        print("\n" + get_text("interrupted"))
        sys.exit(1)
    except Exception as e:
        debug(f"错误: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
