# Read the file content into the STRING_CONTENT variable
file(READ ${INPUT_FILE} STRING_CONTENT)

# Generate the header file
configure_file("${TEMPLATE_FILE}" "${OUTPUT_FILE}" @ONLY)