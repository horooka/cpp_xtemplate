// Hardcoded config example
const char* xtemplate_hardcoded = R"(
hardcoded stdout
std::string &output_str
hardcoded, cout, reference
<TEMPLATE_BODY>
std::cout << $1 << std::endl;
</TEMPLATE_BODY>

hardcoded stderr
std::string &output_str
hardcoded, cerr, reference
<TEMPLATE_BODY>
std::cerr << $1 << std::endl;
</TEMPLATE_BODY>
)";
