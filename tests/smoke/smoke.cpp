#include <QString>

int main()
{
    const QString sample {QStringLiteral("GnotePad")};
    return sample.isEmpty() ? 1 : 0;
}
