FROM almalinux:9

RUN dnf install -y epel-release && \
    dnf config-manager --set-enabled crb && \
    dnf makecache && \
    dnf install -y meson ninja-build gtk4-devel pkgconf-pkg-config gcc make rpm-build tar git && \
    dnf clean all

WORKDIR /build
COPY . .

RUN mkdir -p ~/rpmbuild/SOURCES ~/rpmbuild/SPECS && \
    cd / && \
    mv build same-gnome-1.0.0 && \
    tar czf ~/rpmbuild/SOURCES/same-gnome-1.0.0.tar.gz --exclude=.git --exclude=builddir --exclude=debian same-gnome-1.0.0/ && \
    cp /same-gnome-1.0.0/same-gnome.spec ~/rpmbuild/SPECS/ && \
    rpmbuild -ba ~/rpmbuild/SPECS/same-gnome.spec

CMD ["bash", "-c", "cp ~/rpmbuild/RPMS/x86_64/*.rpm /output/ 2>/dev/null || echo 'Build artifacts not found'"]