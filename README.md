## Compile and Run

1. Prerequisites：

    - Ubuntu 20.04:

      ```bash
      sudo apt install gir1.2-goocanvas-2.0 gir1.2-gtk-3.0 libgirepository1.0-dev python3-dev python3-gi python3-gi-cairo python3-pip python3-pygraphviz python3-pygccxml
      sudo pip3 install kiwi
      ```

2. Pull Code：

   ```bash
   git clone https://github.com/Enidsky/mircc.git
   git submodule update --init --recursive
   ```

3. Compile and install
    - debug(slow but have log)
      ```bash
      cd ndnSIM-template/ns3
      ./waf configure -d debug --enable-examples
      sudo ./waf install  
      ```
    - optimized(fast but have no log)
      ```bash
      cd ndnSIM-template/ns3
      ./waf configure -d optimized --enable-examples
      sudo ./waf install
      ```