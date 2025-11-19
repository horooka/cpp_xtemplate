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

void make_hardcoded_template(Glib::RefPtr<Gtk::ListStore> liststore_templates,
                             const TemplateCols &template_cols,
                             const std::string &name,
                             const std::string &vars_names,
                             const std::string &tags, const std::string &body) {
    Gtk::TreeModel::Row new_row = *liststore_templates->append();
    new_row[template_cols.name] = name;
    new_row[template_cols.is_hardcoded] = true;
    new_row[template_cols.vars_names] = split_by_comma(vars_names);
    new_row[template_cols.tags] = split_by_comma(tags);
    new_row[template_cols.body] = body;
}

int parse_config(const std::string &config_path,
                 Glib::RefPtr<Gtk::ListStore> liststore_tempalates,
                 const TemplateCols &template_cols) {
    std::ifstream file(config_path);
    if (!file.is_open())
        return errno;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        Gtk::TreeModel::Row new_row = *liststore_tempalates->append();
        new_row[template_cols.name] = line;

        if (!std::getline(file, line))
            continue;
        new_row[template_cols.vars_names] = split_by_comma(line);

        if (!std::getline(file, line))
            continue;
        new_row[template_cols.tags] = split_by_comma(line);

        std::string body;
        bool template_body_parsing = false;
        while (std::getline(file, line)) {
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

        std::vector<std::string> var_names = row[template_cols.vars_names];
        for (size_t i = 0; i < var_names.size(); ++i) {
            file << var_names[i];
            if (i + 1 < var_names.size())
                file << ", ";
        }
        file << "\n";

        std::vector<std::string> tags = row[template_cols.tags];
        for (size_t i = 0; i < tags.size(); ++i) {
            file << tags[i];
            if (i + 1 < tags.size())
                file << ", ";
        }
        file << "\n";

        std::string body = static_cast<Glib::ustring>(row[template_cols.body]);
        file << "<TEMPLATE_BODY>\n" << trim(body) << "\n</TEMPLATE_BODY>\n\n";
    }
    return 0;
}
