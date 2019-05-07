
module fapi_encode_tests

    use odc
    use odc_config
    use, intrinsic :: iso_fortran_env
    implicit none

    integer(8), parameter :: col1_data(7) = [1, 2, 3, 4, 5, 6, 7]
    integer(8), parameter :: col2_data(7) = [0, 0, 0, 0, 0, 0, 0]
    integer(8), parameter :: col3_data(7) = [73, 73, 73, 73, 73, 73, 73]
    real(8), parameter :: col4_data(7) = [1.432, 1.432, 1.432, 1.432, 1.432, 1.432, 1.432]
    integer(8), parameter :: col5_data(7) = [-17, -7, -7, 999999, 1, 4, 4]
    character(16), parameter :: col6_data(7) = [character(16) :: "aoeu", "aoeu", "abcdefghijkl", "None", "boo", "squiggle", "a"]
    character(8), parameter :: col7_data(7) = [character(8) :: "abcd", "abcd", "abcd", "abcd", "abcd", "abcd", "abcd"]
    real(8), parameter :: col8_data(7) = [2.345, 2.345, 2.345, 2.345, 2.345, 2.345, 2.345]
    real(8), parameter :: col9_data(7) = [999.99, 888.88, 777.77, 666.66, 999999.0, 444.44, 333.33]
    real(8), parameter :: col10_data(7) = [999.99, 888.88, 777.77, 666.66, 999999.0, 444.44, 333.33]
    integer(8), parameter :: col11_data(7) = [1, 999999, 3, 4, 5, 999999, 8]
    integer(8), parameter :: col12_data(7) = [-512, 999999, 3, 7623, -22000, 999999, 7]
    integer(8), parameter :: col13_data(7) = [-1234567, 8765432, 999999, 22, 2222222, -81222323, 999999]
    integer(8), parameter :: col14_data(7) = [999999, 999999, 999999, 999999, 999999, 999999, 999999]

contains

    ! TODO: Test missing doubles
    ! TODO: Test missing REAL
    ! TODO: Encoding NaN?

    subroutine check_call(err, msg, success)
        integer, intent(in) :: err
        character(*), intent(in) :: msg
        logical, intent(inout) :: success

        if (err /= ODC_SUCCESS) then
            write(error_unit, *) 'Failed API call: ', msg
            write(error_unit, *) 'Error: ', odc_error_string(err)
            success = .false.
        end if
    end subroutine

    subroutine check_decoded_column_major(data, success)
        real(8), intent(in) :: data(:, :)
        integer :: row
        character(16) :: str16
        character(8) :: str8
        logical, intent(inout) :: success

        if (size(data, 1) /= 7 .or. size(data, 2) /= 15) then
            write(error_unit, *) 'did not get data shape [7, 15]'
            success = .false.
        end if

        if (any(col1_data /= data(:, 1))) then
            write(error_unit, *) 'Col 1 differs: ', col1_data, ' vs ', data(:, 1)
            success = .false.
        end if

        if (any(col2_data /= data(:, 2))) then
            write(error_unit, *) 'Col 2 differs: ', col2_data, ' vs ', data(:, 2)
            success = .false.
        end if

        if (any(col3_data /= data(:, 3))) then
            write(error_unit, *) 'Col 3 differs: ', col3_data, ' vs ', data(:, 3)
            success = .false.
        end if

        if (any(abs(col4_data - data(:, 4)) > 1.0e-10)) then
            write(error_unit, *) 'Col 4 differs: ', col4_data, ' vs ', data(:, 4)
            success = .false.
        end if

        if (any(col5_data /= data(:, 5))) then
            write(error_unit, *) 'Col 5 differs: ', col5_data, ' vs ', data(:, 5)
            success = .false.
        end if

        do row = 1, 7
            if (trim(col6_data(row)) /= trim(transfer(data(row, 6:7), str16))) then
                write(error_unit, *) 'Col 6 differs: ', trim(col6_data(row)), ' vs ', &
                                     transfer(data(row, 6:7), str16)
                success = .false.
            end if

            if (trim(col7_data(row)) /= trim(transfer(data(row, 8), str8))) then
                write(error_unit, *) 'Col 7 differs: ', trim(col6_data(row)), ' vs ', &
                                     transfer(data(row, 8), str8)
                success = .false.
            end if
        end do

        if (any(abs(col8_data - data(:, 9)) > 1.0e-10)) then
            write(error_unit, *) 'Col 8 differs: ', col8_data, ' vs ', data(:, 9)
            success = .false.
        end if

        if (any(abs(col9_data - data(:, 10)) > 1.0e-10)) then
            write(error_unit, *) 'Col 9 differs: ', col9_data, ' vs ', data(:, 10)
            success = .false.
        end if

        if (any(abs(col10_data - data(:, 11)) > 1.0e-10)) then
            write(error_unit, *) 'Col 10 differs: ', col10_data, ' vs ', data(:, 11)
            success = .false.
        end if

        if (any(col11_data /= data(:, 12))) then
            write(error_unit, *) 'Col 11 differs: ', col11_data, ' vs ', data(:, 12)
            success = .false.
        end if

        if (any(col12_data /= data(:, 13))) then
            write(error_unit, *) 'Col 12 differs: ', col12_data, ' vs ', data(:, 13)
            success = .false.
        end if

        if (any(col13_data /= data(:, 14))) then
            write(error_unit, *) 'Col 13 differs: ', col13_data, ' vs ', data(:, 14)
            success = .false.
        end if

        if (any(col14_data /= data(:, 15))) then
            write(error_unit, *) 'Col 14 differs: ', col14_data, ' vs ', data(:, 15)
            success = .false.
        end if
    end subroutine

    function construct_data_column_major() result(data)
        real(8) :: data(7, 15)
        integer :: row

        data(:, 1) = col1_data
        data(:, 2) = col2_data
        data(:, 3) = col3_data
        data(:, 4) = col4_data
        data(:, 5) = col5_data
        do row = 1, 7
            data(row, 6:7) = transfer(col6_data(row), 1.0_8, 2)
            data(row, 8) = transfer(col7_data(row), 1.0_8)
        end do
        data(:, 9) = col8_data
        data(:, 10) = col9_data
        data(:, 11) = col10_data
        data(:, 12) = col11_data
        data(:, 13) = col12_data
        data(:, 14) = col13_data
        data(:, 15) = col14_data
    end function

    subroutine initialise_encoder(encoder, success)
        type(odc_encoder) :: encoder
        logical, intent(inout) :: success

        call check_call(encoder%initialise(), "initialise encoder", success)

        call check_call(encoder%add_column("col1", ODC_INTEGER), "add col1", success)
        call check_call(encoder%add_column("col2", ODC_INTEGER), "add col2", success)
        call check_call(encoder%add_column("col3", ODC_BITFIELD), "add col3", success)
        call check_call(encoder%add_column("col4", ODC_DOUBLE), "add col4", success)
        call check_call(encoder%add_column("col5", ODC_INTEGER), "add col5", success)
        call check_call(encoder%add_column("col6", ODC_STRING), "add col6", success)
        call check_call(encoder%add_column("col7", ODC_STRING), "add col7", success)
        call check_call(encoder%add_column("col8", ODC_REAL), "add col8", success)
        call check_call(encoder%add_column("col9", ODC_DOUBLE), "add col9", success)
        call check_call(encoder%add_column("col10", ODC_REAL), "add col10", success)
        call check_call(encoder%add_column("col11", ODC_BITFIELD), "add col11", success)
        call check_call(encoder%add_column("col12", ODC_INTEGER), "add col12", success)
        call check_call(encoder%add_column("col13", ODC_INTEGER), "add col13", success)
        call check_call(encoder%add_column("col14", ODC_INTEGER), "add col14", success)

        call check_call(encoder%column_set_attrs(6, element_size_doubles=2), "column attrs", success)

        call check_call(encoder%column_add_bitfield(11, "bf1", 3), "add bitfield 1", success)
        call check_call(encoder%column_add_bitfield(11, "bf2", 2), "add bitfield 2", success)
        call check_call(encoder%column_add_bitfield(11, "bf3", 1), "add bitfield 3", success)
    end subroutine

    function test_encode_column_major() result(success)

        real(8) :: data(7, 15)
        integer :: outunit
        integer(8) :: bytes_written
        type(odc_encoder) :: encoder
        logical :: success
        success = .true.

        data = construct_data_column_major()

        call check_decoded_column_major(data, success)
        call initialise_encoder(encoder, success)

        call check_call(encoder%set_data(data), "set encoder data", success)

        open(newunit=outunit, file='testout.odb', access='stream', form='unformatted')
        call check_call(encoder%encode(outunit, bytes_written), "do encode", success)
        close(outunit)

        call check_call(encoder%free(), "free encoder", success)

        call check_encoded_odb('testout2.odb', success)

    end function

    function test_encode_row_major() result(success)

        real(8) :: data(15, 7)
        integer :: row, outunit
        integer(8) :: bytes_written
        type(odc_encoder) :: encoder
        logical :: success
        success = .true.

        data = transpose(construct_data_column_major())

        call check_decoded_column_major(transpose(data), success)
        call initialise_encoder(encoder, success)

        call check_call(encoder%set_data(data, column_major=.false.), "set encoder data", success)

        open(newunit=outunit, file='testout2.odb', access='stream', form='unformatted')
        call check_call(encoder%encode(outunit, bytes_written), "do encode", success)
        close(outunit)

        call check_call(encoder%free(), "free encoder", success)

        call check_encoded_odb('testout2.odb', success)

    end function

    subroutine check_encoded_odb(path, success)
        character(*), intent(in) :: path
        logical, intent(inout) :: success

        type(odc_reader) :: reader
        type(odc_frame) :: frame
        type(odc_decoder) :: decoder
        real(8), pointer :: data(:,:)
        logical :: column_major
        integer :: err
        integer(8) :: nrows

        call check_call(reader%open_path(path), "open " // path, success)
        call check_call(frame%initialise(reader), "initialise frame", success)

        ! We are expecting one frame

        call check_call(frame%next(), "get first frame", success)

        ! Decode the data

        call check_call(decoder%initialise(), "initialise decoder", success)
        call check_call(decoder%defaults_from_frame(frame), "defaults from frame", success)
        call check_call(decoder%decode(frame, nrows), "decode", success)
        call check_call(decoder%data(data, column_major), "get data", success)

        if (.not. column_major) then
            write(error_unit, *) 'expected column major'
            success = .false.
        end if

        call check_decoded_column_major(data, success)
        call check_call(decoder%free(), "free decoder", success)

        ! And iterations done

        err = frame%next()
        if (err /= ODC_ITERATION_COMPLETE) then
            write(error_unit, *) 'expected iteration complete'
            success = .false.
        end if

        ! Cleanup

        call check_call(frame%free(), "free frame", success)
        call check_call(reader%close(), "free frame", success)

    end subroutine

    !funcion test_encode_integers() result(success)
    !    logical :: success
    !    success = .true.
    !end function

    !function test_encode_columns() result(success)
    !    logical :: success
    !    success = .true.
    !end function

end module


program fapi_general

    use fapi_encode_tests
    implicit none

    logical :: success

    success = .true.
    call check_call(odc_initialise_api(), "initialise api", success)
    call check_call(odc_set_missing_integer(999999_8), "set missing integer", success)
    call check_call(odc_set_missing_double(999999.0_8), "set missing double", success)

    success = test_encode_column_major() .and. success
    success = test_encode_row_major() .and. success
    !success = test_encode_integers() .and. success
    !success = test_encode_columns() .and. success

    if (.not. success) stop -1

end program
