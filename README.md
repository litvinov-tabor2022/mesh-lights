# ESP lights

Mesh network for controlling lights (or other peripheries) wirelessly over long distances.

## About

The `painlessMesh` library is used as the core of wireless communication. The main thought is to have one *root* node with a
web interface and here is where the first issue occurs. ESP can create only one AP at a time. Obviously, this AP will be
used for communication inside the mesh network, therefore other communication can not be done through it. A possible
workaround for this problem is to connect the *root* to another AP (another network) that is not associated with the mesh.
This hack has one disadvantage - the operator (respectively AP/another network) must be close to the *root*. The required
behavior implies that the *root* node must be always in the mesh network, otherwise the controlling will have not to work.

## Usage

1. upload slave `pio run -t upload -e 32node2`
2. upload root `pio run -t upload -e 32root`

