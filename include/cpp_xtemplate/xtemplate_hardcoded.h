// Hardcoded config example
const char* xtemplate_hardcoded = R"(
hardcoded stdout
hardcoded, cout, reference
std::string &;output_str
<TEMPLATE_BODY>
std::cout << $1 << std::endl;
</TEMPLATE_BODY>

hardcoded stderr
hardcoded, cerr, reference
std::string &;output_str
<TEMPLATE_BODY>
std::cerr << $1 << std::endl;
</TEMPLATE_BODY>
)";
