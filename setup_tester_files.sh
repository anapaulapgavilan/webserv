#!/bin/bash

# This script creates the required files and directory structure for webserver testing

echo "=============================================="
echo "Welcome to the Environment Setup"
echo "=============================================="
echo ""
echo ""
read -p "Press enter to continue..."
echo ""


# Create YoupiBanane directory structure
echo "Creating YoupiBanane directory structure..."

# Remove existing YoupiBanane if it exists
if [ -d "YoupiBanane" ]; then
    echo "Removing existing YoupiBanane directory..."
    rm -rf YoupiBanane
fi

# Create main directory
mkdir -p YoupiBanane

# Create files in YoupiBanane root
echo "Creating main directory files..."
cat > YoupiBanane/youpi.bad_extension << 'EOF'
This is a test file with bad_extension.
Content for testing purposes.
This file should be served by the webserver when requested.
EOF

cat > YoupiBanane/youpi.bla << 'EOF'
This is a test file with .bla extension.
This file should trigger CGI execution when accessed via POST request.
Content for CGI testing.
EOF

# Create nop subdirectory and its files
echo "Creating nop subdirectory..."
mkdir -p YoupiBanane/nop

cat > YoupiBanane/nop/youpi.bad_extension << 'EOF'
This is a test file in the nop subdirectory.
File with bad_extension in nop directory.
Used for directory traversal testing.
EOF

cat > YoupiBanane/nop/other.pouic << 'EOF'
This is another test file in nop subdirectory.
File with .pouic extension for testing purposes.
Random content for webserver testing.
EOF

# Create Yeah subdirectory and its files
echo "Creating Yeah subdirectory..."
mkdir -p YoupiBanane/Yeah

cat > YoupiBanane/Yeah/not_happy.bad_extension << 'EOF'
This is a test file in the Yeah subdirectory.
File with bad_extension in Yeah directory.
Content for negative testing scenarios.
EOF

echo ""
echo "Directory structure created successfully!"
echo ""

# Display the created structure
echo "Created directory structure:"
echo "YoupiBanane/"
echo "├── youpi.bad_extension"
echo "├── youpi.bla"
echo "├── nop/"
echo "│   ├── youpi.bad_extension"
echo "│   └── other.pouic"
echo "└── Yeah/"
echo "    └── not_happy.bad_extension"
echo ""

# Display file contents
echo "File contents preview:"
echo ""

echo "=== YoupiBanane/youpi.bad_extension ==="
cat YoupiBanane/youpi.bad_extension
echo ""

echo "=== YoupiBanane/youpi.bla ==="
cat YoupiBanane/youpi.bla
echo ""

echo "=== YoupiBanane/nop/youpi.bad_extension ==="
cat YoupiBanane/nop/youpi.bad_extension
echo ""

echo "=== YoupiBanane/nop/other.pouic ==="
cat YoupiBanane/nop/other.pouic
echo ""

echo "=== YoupiBanane/Yeah/not_happy.bad_extension ==="
cat YoupiBanane/Yeah/not_happy.bad_extension
echo ""

read -p "Press enter to continue..."
echo ""

# Display configuration requirements
echo "=============================================="
echo "CONFIGURATION REQUIREMENTS"
echo "=============================================="
echo ""
echo "Setup the configuration file as follow:"
echo ""
echo "- / must answer to GET request ONLY"
echo "- /put_test/* must answer to PUT request and save files to a directory of your choice"
echo "- any file with .bla as extension must answer to POST request by calling the cgi_test executable"
echo "- /post_body must answer anything to POST request with a maxBody of 100"
echo "- /directory/ must answer to GET request and the root of it would be the repository YoupiBanane"
echo "  and if no file are requested, it should search for youpi.bad_extension files"
echo ""


echo "=============================================="
echo "SETUP COMPLETE"
echo "=============================================="
echo ""
echo "The test environment has been successfully created!"
echo "You can now configure your webserver and run the tests."
echo ""
echo "Directory created: $(pwd)/YoupiBanane"
echo "All required files have been created with appropriate content."
echo ""