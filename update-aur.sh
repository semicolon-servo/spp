#!/bin/bash

set -e

echo "Updating AUR package..."

if [ ! -f "PKGBUILD" ]; then
    echo "Error: PKGBUILD not found. Run this script from the repo root."
    exit 1
fi

# Store current branch
CURRENT_BRANCH=$(git branch --show-current)

echo "Regenerating .SRCINFO..."
makepkg --printsrcinfo > .SRCINFO

# Save files to temp location
TMP_DIR=$(mktemp -d)
cp PKGBUILD "$TMP_DIR/"
cp .SRCINFO "$TMP_DIR/"

if git show-ref --verify --quiet refs/heads/aur; then
    echo "Switching to aur branch..."
    git checkout aur
else
    echo "Creating aur branch..."
    git checkout --orphan aur
fi

# Remove everything from index
git rm -rf --cached . 2>/dev/null || true

# Copy files back
cp "$TMP_DIR/PKGBUILD" .
cp "$TMP_DIR/.SRCINFO" .

# Clean up temp dir
rm -rf "$TMP_DIR"

# Remove files from index and add them fresh
git rm --cached PKGBUILD .SRCINFO 2>/dev/null || true
git add PKGBUILD .SRCINFO

# Verify files are staged
if ! git ls-files --cached PKGBUILD .SRCINFO | grep -q PKGBUILD || ! git ls-files --cached PKGBUILD .SRCINFO | grep -q .SRCINFO; then
    echo "Error: Failed to stage PKGBUILD and .SRCINFO"
    git checkout $CURRENT_BRANCH 2>/dev/null || git checkout master
    exit 1
fi

# Check if there are actual changes to commit
if git diff --cached --quiet && [ $(git ls-tree -r HEAD --name-only 2>/dev/null | wc -l) -eq 2 ]; then
    echo "No changes to commit."
else
    echo "Committing changes..."
    git commit -m "Update AUR package files"
fi

echo "Pushing to AUR..."
git push aur aur:master --force

echo "Switching back to $CURRENT_BRANCH..."
git checkout $CURRENT_BRANCH

echo "AUR package updated successfully!"
