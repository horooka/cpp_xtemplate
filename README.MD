Tool for using and creating clipboard templates

**Features**

- Storing of templates in a file at ~/.config/xtemplate.txt
- Opportunity to hardcode config in include/cpp_xtemplate/xtemplate_hardcoded.h file

**Template file format**  
Templates records are separated by empty line, template body is declared with "<TEMPLATE_BODY>" tag to store empty lines within

- first line is the template name
- third line is the comma separated template's tags
- second line is the comma separated template's vars records, each records is a colon separated var type and var name
- rest of lines is the template's body
