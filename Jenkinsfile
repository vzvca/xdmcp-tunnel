

pipeline {
    options {
	/* ansiColor('xterm') */
	buildDiscarder(logRotator(artifactDaysToKeepStr: '', artifactNumToKeepStr:'', daysToKeepStr: '', numToKeepStr: '5'))
	disableConcurrentBuilds()
    }

    environment {
	REPOSITORY="https://github.com/vzvca/xdmcp-tunnel"
	BRANCH="master"
    }

    agent {
        label "amd64"
    }

    stages {
	stage('git-clone'){
	    steps {
		checkout(  changelog: false,
			   scm :[
				 $class: 'GitSCM',
				 branches: [[name: "*/${BRANCH}"]],
				 doGenerateSubmoduleConfigurations: false,
				 extensions: [
					      [ $class: 'SubmoduleOption',
						disableSubmodules: false,
						parentCredentials: true,
						recursiveSubmodules: true,     // Equivalent a l'option git --recursive
						trackingSubmodules: false,     // Equivalent a l'option git --remote
						timeout: 30],

					      [$class: 'CloneOption', depth: 1, noTags: false, reference: '', shallow: true]
					     ],

				 submoduleCfg: [],
				 userRemoteConfigs: [[
						      credentialsId: 'vzvca',
						      url: "${REPOSITORY}.git"
						     ]],      ])
	    }
	}

	stage('exec'){
	    steps {
		sh 'make'
	    }
	}
    }
}
