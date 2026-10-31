#include "Parser.h"
#include <cstring>
using namespace std;

map<string, Atom> atoms;
vector<string> order;

static string replaceAll(string s, const string& a, const string& b) {
    for (size_t p = 0; (p = s.find(a, p)) != string::npos; p += b.size()) s.replace(p, a.size(), b);
    return s;
}
static bool startsWith(const string& s, const string& p) { return s.compare(0, p.size(), p) == 0; }

string trim(string s) {
    auto bad = [](unsigned char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ',' || c == ';' || c == '.' || c == '!' || c == '?';
    };
    while (!s.empty() && bad(s.back())) s.pop_back();
    size_t i = 0; while (i < s.size() && bad(s[i])) i++;
    return s.substr(i);
}

string lowerEs(string s) {
    const char* up[] = { "Á","É","Í","Ó","Ú","Ñ","Ü" };
    const char* lo[] = { "á","é","í","ó","ú","ñ","ü" };
    for (int i = 0; i < 7; i++) s = replaceAll(s, up[i], lo[i]);
    for (auto& c : s) if ((unsigned char)c < 128) c = (char)tolower((unsigned char)c);
    s = replaceAll(replaceAll(s, "¡", ""), "¿", "");
    while (s.find("  ") != string::npos) s = replaceAll(s, "  ", " ");
    return s;
}

static string keyOf(string s) {
    const char* a[] = { "á","é","í","ó","ú","ü" };
    const char* b[] = { "a","e","i","o","u","u" };
    for (int i = 0; i < 6; i++) s = replaceAll(s, a[i], b[i]);
    return trim(s);
}

// Corta en la primera aparición de " w " (palabra completa)
static bool cut(const string& s, const string& w, string& a, string& b) {
    size_t p = s.find(" " + w + " ");
    if (p == string::npos) return false;
    a = s.substr(0, p); b = s.substr(p + w.size() + 2);
    return true;
}

static const string LET = "pqrstuvwxyzabcdefghijklmno";

static P atomOf(const string& t0) {
    string t = trim(t0); if (t.empty()) t = "…";
    string k = keyOf(t);
    if (!atoms.count(k)) {
        size_t i = atoms.size();
        atoms[k] = { i < 26 ? string(1, LET[i]) : "p" + to_string(i - 25), t };
        order.push_back(k);
    }
    return atomF(atoms[k].n);
}

// De menor a mayor precedencia: negación de frase completa, ↔, →, ∨, ∧, ¬ de átomo
P parse(string s) {
    s = trim(s);
    for (const char* p : { "no es verdad que ", "no es cierto que ", "es falso que ", "no ocurre que ", "no es el caso que " })
        if (startsWith(s, p)) return neg(parse(s.substr(strlen(p))));
    string a, b;
    if (cut(s, "si y solo si", a, b)) { P l = parse(a); return bin(tIff, l, parse(b)); }
    if (cut(s, "solo si", a, b))      { P l = parse(a); return bin(tImp, l, parse(b)); }
    if (startsWith(s, "si ")) {
        size_t p = s.find(" entonces ");
        if (p != string::npos) { P l = parse(s.substr(3, p - 3)); return bin(tImp, l, parse(s.substr(p + 10))); }
        p = s.find(", ");
        if (p != string::npos) { P l = parse(s.substr(3, p - 3)); return bin(tImp, l, parse(s.substr(p + 2))); }
    }
    if (cut(s, "si", a, b)) { P c = parse(a); P ant = parse(b); return bin(tImp, ant, c); }   // "B si A"
    if (cut(s, "o", a, b) || cut(s, "u", a, b))       { P l = parse(a); return bin(tOr, l, parse(b)); }
    if (cut(s, "y", a, b) || cut(s, "pero", a, b))    { P l = parse(a); return bin(tAnd, l, parse(b)); }
    if (startsWith(s, "no ")) return neg(parse(s.substr(3)));
    return atomOf(s);
}
