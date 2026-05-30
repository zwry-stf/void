#pragma once
#include "common.h"

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>


namespace resources {
	struct parsed_file {
		std::vector<std::uint8_t> data;
	};

	bool parse(const std::uint8_t* arr, std::size_t size, int id, parsed_file& out) {
		if (size < sizeof(global_header))
			return false;

		global_header header;
		std::memcpy(
			&header, 
			arr, 
			sizeof(header)
		);

		if (header.num_files == 0u)
			return false;

		for (std::uint32_t i = 0u; i < header.num_files; i++) {
			const std::uint32_t offset = static_cast<std::uint32_t>(sizeof(header) + i * sizeof(file_header));
			if (size < offset + sizeof(file_header))
				return false;

			file_header fheader;
			std::memcpy(
				&fheader,
				arr + offset,
				sizeof(fheader)
			);

			if (fheader.id == id) {
				const std::uint32_t data_start = header.data_pointer + fheader.data_offset;
				const std::uint32_t data_end = data_start + fheader.data_size;
				if (data_end > size || data_start > size) {
					return false;
				}

				out.data.assign(
					arr + data_start,
					arr + data_end
				);
				if (header.use_encryption) {
					std::uint8_t real_key[sizeof(fheader.encryption_key)];
					derive_rc4_key(fheader.encryption_key, real_key);
					rc4_crypt(
						out.data.data(),
						out.data.size(),
						real_key,
						sizeof(real_key)
					);
				}
				return true;
			}
		}

		return false;
	}
}