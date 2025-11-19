Tool for using and creating clipboard templates

**Features**

- storing of templates in a file at ~/.config/xtemplate.txt
- opportunity to hardcode config in include/cpp_xtemplate/xtemplate_hardcoded.h file

**Template file format**  
Templates records are separated by empty line, template body is declared with "<TEMPLATE_BODY>" tag to store empty lines within

- first line is the template name
- second line is the comma separated template's vars names
- third line is the comma separated template's tags
- rest of lines is the template's body
