#include <array>
#include <cassert>
#include <cstdio>
#include <functional>
#include <optional>

#include <fmt/format.h>

struct vec3 {
	float x;
	float y;
	float z;

	constexpr vec3(float x, float y, float z) : x{x}, y{y}, z{z} {}
};

struct vec4 {
	float x;
	float y;
	float z;
	float w;

	constexpr vec4(float x, float y, float z, float w) : x{x}, y{y}, z{z}, w{w} {}
	constexpr vec4(vec3 v, float w) : x{v.x}, y{v.y}, z{v.z}, w{w} {}

	constexpr void normalize() {
		x /= w;
		y /= w;
		z /= w;
		w /= w;
	}
};

typedef std::array<float, 4 * 4> mat4;

constexpr mat4 get_projection(float l, float r, float t, float b, float n, float f) {
	return std::to_array<float>({
		2.0f * n / (r - l), 0,                  (l + r) / (l - r), 0,
		0,                  2.0f * n / (b - t), (t + b) / (t - b), 0,
		0,                  0,                  (f + n) / (f - n), 2.0f * n * f / (n - f),
		0,                  0,                  1.0f,              0
	});
}

constexpr vec4 mat4_mul_vec4(mat4 m, vec4 v) {
	return vec4{
		m[0 * 4 + 0] * v.x + m[0 * 4 + 1] * v.y + m[0 * 4 + 2] * v.z + m[0 * 4 + 3] * v.w,
		m[1 * 4 + 0] * v.x + m[1 * 4 + 1] * v.y + m[1 * 4 + 2] * v.z + m[1 * 4 + 3] * v.w,
		m[2 * 4 + 0] * v.x + m[2 * 4 + 1] * v.y + m[2 * 4 + 2] * v.z + m[2 * 4 + 3] * v.w,
		m[3 * 4 + 0] * v.x + m[3 * 4 + 1] * v.y + m[3 * 4 + 2] * v.z + m[3 * 4 + 3] * v.w
	};
}

constexpr bool inside_triangle(vec4 a, vec4 b, vec4 c, float x, float y) {
	constexpr auto edge = [](vec4 a, vec4 b, float x, float y) -> bool {
		return (x - a.x) * (b.y - a.y) - (y - a.y) * (b.x - a.x) >= 0;
	};
	return edge(b, a, x, y) && edge(c, b, x, y) && edge(a, c, x, y);
}

constexpr float get_z_component(vec4 a, vec4 b, vec4 c, float x, float y) {
	float x_p = (b.y - a.y) * (c.z - a.z) - (c.y - a.y) * (b.z - a.z);
	float y_p = (b.z - a.z) * (c.x - a.x) - (c.z - a.z) * (b.x - a.x);
	float z_p = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
	float w_p = -(x_p * a.x + y_p * a.y + z_p * a.z);
	return (-x_p * x - y_p * y - w_p) / z_p;
}

constexpr float to_cartesian(int coord, int factor) {
	return (coord * 2.0) / (factor - 1) - 1;
}

template <int W, int H, typename char_type>
struct screen {
	using shader_fn_type = std::function<char_type(float)>;

	struct fragment {
		shader_fn_type fn;

		auto operator()(float n) {
			return fn(n);
		}
	} shader;

	std::array<char_type, W * H> buf;
	std::array<float, W * H> depth;
	const int w = W;
	const int h = H;

	constexpr screen(shader_fn_type &&fn) : shader{fn} {
		buf.fill('.');
		depth.fill(0);
	}

	constexpr char_type& at(int x, int y) {
		assert(y * w + x < w * h);
		return buf[y * w + x];
	}

	constexpr std::optional<std::reference_wrapper<char_type>> at(int x, int y, float d) {
		assert(y * w + x < w * h);
		if (depth[y * w + x] + 1 < d) {
			return std::nullopt;
		}
		depth[y * w + x] = d;
		return std::optional{std::reference_wrapper<char_type>{buf[y * w + x]}};
	}

	constexpr void render() {
		std::putc('+', stdout);
		for (int i = 0; i < w; i++) {
			std::putc('-', stdout);
		}
		std::putc('+', stdout);
		std::putc('\n', stdout);
		for (int i = 0; i < h; i++) {
			std::putc('|', stdout);
			for (int j = 0; j < w; j++) {
				std::putc(at(j, i), stdout);
			}
			std::putc('|', stdout);
			std::putc('\n', stdout);
		}
		std::putc('+', stdout);
		for (int i = 0; i < w; i++) {
			std::putc('-', stdout);
		}
		std::putc('+', stdout);
	}

	constexpr void draw(vec3 v1, vec3 v2, vec3 v3) {
		constexpr auto projection = get_projection(-1, 1, -1, 1, 1, 2);

		auto p1 = mat4_mul_vec4(projection, vec4{v1, 1.0});
		auto p2 = mat4_mul_vec4(projection, vec4{v2, 1.0});
		auto p3 = mat4_mul_vec4(projection, vec4{v3, 1.0});

		p1.normalize();
		p2.normalize();
		p3.normalize();

		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				if (inside_triangle(p1, p2, p3, to_cartesian(j, w), to_cartesian(i, h))) {
					auto d = get_z_component(p1, p2, p3, to_cartesian(j, w), to_cartesian(i, h));
					if (auto c = at(j, i, d)) {
						c->get() = shader(d);
					}
				}
			}
		}
	}
};

template<> struct fmt::formatter<vec3> {
	template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const vec3& v, FormatContext& ctx) {
		return format_to(ctx.out(), "x: {} y: {} z: {}", v.x, v.y, v.z);
	}
};

template<> struct fmt::formatter<vec4> {
	template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const vec4& v, FormatContext& ctx) {
		return format_to(ctx.out(), "x: {} y: {} z: {} w: {}", v.x, v.y, v.z, v.w);
	}
};

template<> struct fmt::formatter<mat4> {
	template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const mat4& m, FormatContext& ctx) {
		return format_to(ctx.out(),
			"{} {} {} {}\n{} {} {} {}\n{} {} {} {}\n{} {} {} {}\n",
			m[0 * 4 + 0], m[0 * 4 + 1], m[0 * 4 + 2], m[0 * 4 + 3],
			m[1 * 4 + 0], m[1 * 4 + 1], m[1 * 4 + 2], m[1 * 4 + 3],
			m[2 * 4 + 0], m[2 * 4 + 1], m[2 * 4 + 2], m[2 * 4 + 3],
			m[3 * 4 + 0], m[3 * 4 + 1], m[3 * 4 + 2], m[3 * 4 + 3]
		);
	}
};

int main() {
	const auto shader = [](float n) -> char {
		return '0' + (n + 1.0f) * 5.0f;
	};

	screen<150, 50, char> screen(shader);

	const auto v1 = vec3{ 1.0, -1.0,  1.5};
	const auto v2 = vec3{ 1.0,  1.0,  1.1};
	const auto v3 = vec3{-1.0,  1.0,  1.5};
	const auto v4 = vec3{-1.0, -1.0,  1.9};

	screen.draw(v1, v2, v3);
	screen.draw(v1, v3, v4);
	screen.render();
}
