#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs
from paths import IMG_DIR


def delete_file(filename):
    file_path = os.path.abspath(os.path.join(IMG_DIR, filename))
    # Prevent directory traversal
    if not file_path.startswith(os.path.abspath(IMG_DIR)):
        print("Content-Type: text/plain\n")
        print("Invalid file path.")
        return

    if os.path.isfile(file_path):
        try:
            os.remove(file_path)
            print("Content-Type: text/plain\n")
            print(f"File '{filename}' deleted successfully.")
        except Exception as e:
            print("Content-Type: text/plain\n")
            print(f"Error deleting file: {e}")
    else:
        print("Content-Type: text/plain\n")
        print("File not found.")

def parse_form_data():
    """Parse form data from query string or POST data"""
    query_string = os.environ.get('QUERY_STRING', '')
    request_method = os.environ.get('REQUEST_METHOD', 'DELETE')
    
    if request_method == 'DELETE' and query_string:
        # Parse query string
        parsed_data = parse_qs(query_string)
        return {key: values[0] for key, values in parsed_data.items() if values}
    elif request_method == 'POST':
        content_length = int(os.environ.get('CONTENT_LENGTH', '0'))
        if content_length > 0:
            post_data = sys.stdin.read(content_length)
            parsed_data = parse_qs(post_data)
            return {key: values[0] for key, values in parsed_data.items() if values}
    
    return {}

if __name__ == "__main__":
    # Get filename from form data
    form_data = parse_form_data()
    filename = form_data.get("img", "")

    if filename:
        delete_file(filename)
    else:
        print("Content-Type: text/plain\n")
        print("No filename provided.")
