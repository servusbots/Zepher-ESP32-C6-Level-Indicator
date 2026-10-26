import sys
import requests

def main():
    if len(sys.argv) < 3:
        print("Usage: python download_file.py <url> <destination>")
        sys.exit(1)
    url = sys.argv[1]
    dest = sys.argv[2]
    
    print(f"Connecting to {url} ...")
    resp = requests.get(url, stream=True, timeout=30)
    resp.raise_for_status()
    total = int(resp.headers.get('content-length', 0))
    downloaded = 0
    with open(dest, 'wb') as f:
        for chunk in resp.iter_content(chunk_size=1048576):
            if chunk:
                f.write(chunk)
                downloaded += len(chunk)
                if total > 0:
                    pct = downloaded * 100 // total
                    sys.stdout.write(f"\rDownloading: {downloaded/(1024*1024):.1f}MB / {total/(1024*1024):.1f}MB ({pct}%)")
                    sys.stdout.flush()
    print("\nDownload complete.")

if __name__ == "__main__":
    main()
