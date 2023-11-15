find_package(Git 2.22 REQUIRED)


function(alp_add_git_repository name)
    set(options DO_NOT_ADD_SUBPROJECT)
    set(oneValueArgs URL COMMITISH)
    set(multiValueArgs )
    cmake_parse_arguments(PARSE_ARGV 1 PARAM "${options}" "${oneValueArgs}" "${multiValueArgs}")

    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/${ALP_EXTERN_DIR} )
    set(repo_dir ${CMAKE_SOURCE_DIR}/${ALP_EXTERN_DIR}/${name})
    set(short_repo_dir ${ALP_EXTERN_DIR}/${name})

    set(${name}_SOURCE_DIR "${repo_dir}" PARENT_SCOPE)

    if(EXISTS "${repo_dir}/.git")
        message(STATUS "Updating git repo in ${short_repo_dir}")
        execute_process(COMMAND ${GIT_EXECUTABLE} fetch
            WORKING_DIRECTORY ${repo_dir}
            RESULT_VARIABLE GIT_FETCH_RESULT)
        if (NOT ${GIT_FETCH_RESULT})
            message(STATUS "Fetching ${name} was successfull.")

            execute_process(COMMAND ${GIT_EXECUTABLE} branch --show-current
                WORKING_DIRECTORY ${repo_dir}
                RESULT_VARIABLE GIT_BRANCH_RESULT
                OUTPUT_STRIP_TRAILING_WHITESPACE
                OUTPUT_VARIABLE GIT_BRANCH_OUTPUT)
            if (${GIT_BRANCH_RESULT})
                message(FATAL_ERROR "${repo_dir}: git branch --show-current not successfull")
            endif()
            if (GIT_BRANCH_OUTPUT STREQUAL "")
                execute_process(COMMAND ${GIT_EXECUTABLE} checkout --quiet ${PARAM_COMMITISH}
                    WORKING_DIRECTORY ${repo_dir}
                    RESULT_VARIABLE GIT_CHECKOUT_RESULT)
                if (NOT ${GIT_CHECKOUT_RESULT})
                    message(STATUS "In ${name}, checking out ${PARAM_COMMITISH} was successfull.")
                else()
                    message(FATAL_ERROR "In ${name}, checking out ${PARAM_COMMITISH} was NOT successfull!")
                endif()
            else()
                message(WARNING "${short_repo_dir} is on branch ${GIT_BRANCH_OUTPUT}, leaving it there. "
                    "NOT checking out ${PARAM_COMMITISH}! Use origin/main or similar if you want to stay up-to-date with upstream.")
            endif()
        else()
            message(WARNING "Fetching ${name} was NOT successfull!")
        endif()
    else()
        message(STATUS "Clonging ${PARAM_URL} to ${short_repo_dir}.")
        execute_process(COMMAND ${GIT_EXECUTABLE} clone --recurse-submodules ${PARAM_URL} ${name}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${ALP_EXTERN_DIR}
            RESULT_VARIABLE GIT_CLONE_RESULT)
        if (NOT ${GIT_CLONE_RESULT})
            execute_process(COMMAND ${GIT_EXECUTABLE} checkout --quiet ${PARAM_COMMITISH}
                WORKING_DIRECTORY ${repo_dir}
                RESULT_VARIABLE GIT_CHECKOUT_RESULT)
            if (NOT ${GIT_CHECKOUT_RESULT})
                message(STATUS "Checking out ${PARAM_COMMITISH} was successfull.")
            else()
                message(FATAL_ERROR "In ${name}, checking out ${PARAM_COMMITISH} was NOT successfull!")
            endif()
        else()
            message(FATAL_ERROR "Clonging ${name} was NOT successfull!")
        endif()
    endif()

    if (NOT ${PARAM_DO_NOT_ADD_SUBPROJECT})
        add_subdirectory(${repo_dir} ${CMAKE_BINARY_DIR}/alp_external/${name})
    endif()
endfunction()
