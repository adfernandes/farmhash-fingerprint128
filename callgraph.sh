set -e

CXXFILT="${HOME}/Developer/Toolchains/gcc-arm-none-eabi/bin/arm-none-eabi-c++filt"

cd _build

for file in *.dot ; do
    cat "${file}" | \
        "${CXXFILT}" | \
        sed 's,>,\\>,g; s,-\\>,->,g; s,<,\\<,g' | \
        gawk '/external node/{id=$1} $1 != id' \
    > "${file}.$$"
    mv "${file}.$$" "${file}"
    dot -Tpdf -o$(basename "${file}" .dot).pdf < "${file}" > /dev/null 2>&1
done
