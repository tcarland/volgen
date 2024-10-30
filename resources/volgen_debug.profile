# volgen release profile

export PROFILE="volgen_release"
export TCAMAKE_PROJECT=${TCAMAKE_PROJECT:-$(realpath ..)}
export TCAMAKE_HOME=${TCAMAKE_HOME:-$(realpath ${TCAMAKE_PROJECT}/tcamake)}
export TCAMAKE_PREFIX="${HOME}"
export TCAMAKE_DEBUG=1
