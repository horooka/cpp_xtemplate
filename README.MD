Tool for using and creating clipboard templates

**Features**

- storing of templates in a file at ~/.config/xtemplate.txt
- opportunity to use hardcoded templates which will stored in program and not in a file (via make_hardcoded_template func in\after calling setup_data method inside app class constructor)

**Template file format**  
Templates records are separated by empty line, template body is declared with "<TEMPLATE_BODY>" tag to store empty lines

- first line is the template name
- second line is the comma separated template's vars names
- third line is the comma separated template's tags
- rest of lines is the template's body
