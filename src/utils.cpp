#include "cpp_xtemplate/cpp_xtemplate.hpp"
#include <fstream>
#include <gtkmm.h>
#include <sstream>

std::string trim(const std::string &s) {
    auto start = std::find_if_not(s.begin(), s.end(), ::isspace);
    auto end = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
    if (start >= end)
        return std::string{};
    return std::string(start, end);
}

std::vector<std::string> split_by_comma(const std::string &s) {
    std::vector<std::string> result;
    std::istringstream stream(s);
    std::string token;
    while (std::getline(stream, token, ','))
        result.push_back(trim(token));
    return result;
}

void set_margin(Gtk::Widget &widget, int margin_horizontal,
                int margin_vertical) {
    widget.set_margin_top(margin_vertical);
    widget.set_margin_left(margin_horizontal);
    widget.set_margin_right(margin_horizontal);
    widget.set_margin_bottom(margin_vertical);
};

int parse_content(const std::string &content,
                  Glib::RefPtr<Gtk::ListStore> liststore_tempalates,
                  const TemplateCols &template_cols, bool is_hardocded) {
    std::stringstream ss_content(content);
    std::string line;
    while (std::getline(ss_content, line)) {
        if (line.empty())
            continue;

        Gtk::TreeModel::Row new_row = *liststore_tempalates->append();
        new_row[template_cols.is_hardcoded] = is_hardocded;
        new_row[template_cols.name] = line;
        if (!std::getline(ss_content, line))
            continue;
        new_row[template_cols.tags] = split_by_comma(line);
        if (!std::getline(ss_content, line))
            continue;
        std::vector<std::string> vars_records = split_by_comma(line);
        std::vector<std::string> vars_names, vars_types;
        for (const std::string &var_record : vars_records) {
            size_t pos = var_record.rfind(';');
            if (pos != std::string::npos) {
                vars_types.push_back(trim(var_record.substr(0, pos)));
                vars_names.push_back(trim(var_record.substr(pos + 1)));
            } else {
                vars_types.push_back("Not specified");
                vars_names.push_back(trim(var_record));
            }
        }
        new_row[template_cols.vars_types] = vars_types;
        new_row[template_cols.vars_names] = vars_names;

        std::string body;
        bool template_body_parsing = false;
        while (std::getline(ss_content, line)) {
            if (line.empty() && !template_body_parsing)
                break;
            if (line == "<TEMPLATE_BODY>") {
                template_body_parsing = true;
                continue;
            }
            if (line == "</TEMPLATE_BODY>") {
                template_body_parsing = false;
                continue;
            }
            if (!body.empty())
                body += "\n";
            body += line;
        }
        new_row[template_cols.body] = body;
    }
    return 0;
}

int parse_config(const std::string &config_path,
                 Glib::RefPtr<Gtk::ListStore> liststore_tempalates,
                 const TemplateCols &template_cols) {
    std::ifstream file(config_path, std::ios::in);
    if (!file.is_open())
        return errno;
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return parse_content(content, liststore_tempalates, template_cols, false);
}

int parse_config_hardcoded(const std::string &content,
                           Glib::RefPtr<Gtk::ListStore> liststore_tempalates,
                           const TemplateCols &template_cols) {
    return parse_content(content, liststore_tempalates, template_cols, true);
}

int write_config(const std::string &config_path,
                 const Glib::RefPtr<Gtk::ListStore> &liststore_tempalates,
                 const TemplateCols &template_cols) {
    std::ofstream file(config_path);
    if (!file.is_open())
        return errno;
    for (const Gtk::TreeModel::Row &row : liststore_tempalates->children()) {
        if (row[template_cols.is_hardcoded])
            continue;
        std::string name = static_cast<Glib::ustring>(row[template_cols.name]);
        file << trim(name) << "\n";

        std::vector<std::string> tags = row[template_cols.tags];
        for (size_t i = 0; i < tags.size(); ++i) {
            file << tags[i];
            if (i + 1 < tags.size())
                file << ", ";
        }
        file << "\n";

        std::vector<std::string> vars_types = row[template_cols.vars_types];
        std::vector<std::string> vars_names = row[template_cols.vars_names];
        for (size_t i = 0; i < vars_names.size(); ++i) {
            file << vars_types[i] << "; ";
            file << vars_names[i];
            if (i + 1 < vars_names.size())
                file << ", ";
        }
        file << "\n";

        std::string body = static_cast<Glib::ustring>(row[template_cols.body]);
        file << "<TEMPLATE_BODY>\n" << trim(body) << "\n</TEMPLATE_BODY>\n\n";
    }
    return 0;
}
