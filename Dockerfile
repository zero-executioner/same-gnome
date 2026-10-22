FROM almalinux:9

RUN dnf install -y epel-release && \
    dnf config-manager --set-enabled crb && \
    dnf makecache && \
    dnf install -y meson ninja-build gtk4-devel pkgconf-pkg-config gcc make rpm-build tar git && \
    dnf clean all

WORKDIR /build
COPY . .

RUN mkdir -p ~/rpmbuild/SOURCES ~/rpmbuild/SPECS && \
    cp /build/same-gnome.spec ~/rpmbuild/SPECS/ && \
    cd / && \
    mv build same-gnome-1.0.0 && \
    tar czf ~/rpmbuild/SOURCES/same-gnome-1.0.0.tar.gz same-gnome-1.0.0/ && \
    rpmbuild -ba ~/rpmbuild/SPECS/same-gnome.spec

CMD ["bash", "-c", "cp ~/rpmbuild/RPMS/x86_64/*.rpm /output/ 2>/dev/null || echo 'Build artifacts not found'"]