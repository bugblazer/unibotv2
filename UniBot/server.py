#!/usr/bin/env python3
import http.server
import socketserver
import urllib.request
import urllib.error
import json
from urllib.parse import urlparse, parse_qs

PORT = 5000
BACKEND_URL = "http://127.0.0.1:18080"

class ProxyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory="webgui", **kwargs)
    
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
    
    def do_POST(self):
        if self.path.startswith('/api/'):
            self.proxy_request('POST')
        else:
            self.send_error(404)
    
    def do_PUT(self):
        if self.path.startswith('/api/'):
            self.proxy_request('PUT')
        else:
            self.send_error(404)
    
    def do_DELETE(self):
        if self.path.startswith('/api/'):
            self.proxy_request('DELETE')
        else:
            self.send_error(404)
    
    def do_GET(self):
        if self.path.startswith('/api/'):
            self.proxy_request('GET')
        else:
            super().do_GET()
    
    def proxy_request(self, method):
        # Remove /api prefix and forward to backend
        backend_path = self.path[4:]  # Remove '/api'
        url = f"{BACKEND_URL}{backend_path}"
        
        # Read request body if present
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length) if content_length > 0 else None
        
        try:
            # Create request
            req = urllib.request.Request(url, data=body, method=method)
            if body:
                req.add_header('Content-Type', 'application/json')
            
            # Forward request to backend
            with urllib.request.urlopen(req) as response:
                response_data = response.read()
                
                # Send response back to client
                self.send_response(response.status)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(response_data)
                
        except urllib.error.HTTPError as e:
            self.send_response(e.code)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            if e.fp:
                self.wfile.write(e.fp.read())
        except Exception as e:
            print(f"Proxy error: {e}")
            self.send_error(502, f"Bad Gateway: {str(e)}")

with socketserver.TCPServer(("0.0.0.0", PORT), ProxyHTTPRequestHandler) as httpd:
    print(f"Serving on port {PORT} with proxy to {BACKEND_URL}")
    httpd.serve_forever()
